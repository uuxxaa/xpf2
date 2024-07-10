#pragma once
#include <vector>
#include <math/v2_t.h>
#include <list>
#include <algorithm>
#include "polygon.h"
#include <math/xpfmath.h>
#include <limits>

namespace xpf {

struct Vertex
{
    v2_t position;
    int index;
    bool IsConvex = false;
};

struct HoleData
{
    size_t holeIndex;
    size_t bridgeIndex;
    v2_t bridgePoint;
};

// Handles triangulation of given polygon using the 'ear-clipping' algorithm.
// The implementation is based on the following paper:
// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
class Triangulation
{
public:
    std::list<Vertex> vertsInClippedPolygon;
    std::vector<size_t> tris;
    size_t triIndex;

public:
    Triangulation(const Polygon& polygon) {
        size_t numHoleToHullConnectionVerts = 2 * polygon.numHoles;
        size_t totalNumVerts = polygon.numPoints + numHoleToHullConnectionVerts;
        tris.resize((totalNumVerts - 2) * 3);
        vertsInClippedPolygon = GenerateVertexList(polygon);
    }


    // Creates a linked list of all vertices in the polygon, with the hole vertices joined to the hull at optimal points.
    std::list<Vertex> GenerateVertexList(const Polygon& polygon)
    {
        std::list<Vertex> vertexList;
        std::list<Vertex>::iterator iter = vertexList.begin();

        // Add all hull points to the linked list
        for (int i = 0; i < (int)polygon.numHullPoints; i++)
        {
            int prevPointIndex = (i - 1 + polygon.numHullPoints) % polygon.numHullPoints;
            int nextPointIndex = (i + 1) % polygon.numHullPoints;

            bool vertexIsConvex = IsConvex(polygon.points[prevPointIndex], polygon.points[i], polygon.points[nextPointIndex]);
            Vertex currentHullVertex{polygon.points[i], i, vertexIsConvex};

            if (vertexList.empty())
                iter = vertexList.insert(vertexList.begin(), currentHullVertex);
            else
                iter = vertexList.insert(iter, currentHullVertex);
        }

        // Process holes:
        std::vector<HoleData> sortedHoleData;
        for (size_t holeIndex = 0; holeIndex < polygon.numHoles; holeIndex++)
        {
            // Find index of rightmost point in hole. This 'bridge' point is where the hole will be connected to the hull.
            v2_t holeBridgePoint{std::numeric_limits<float>::min(), 0};
            size_t holeBridgeIndex = 0;
            for (size_t i = 0; i < polygon.numPointsPerHole[holeIndex]; i++)
            {
                if (polygon.GetHolePoint(i, holeIndex).x > holeBridgePoint.x)
                {
                    holeBridgePoint = polygon.GetHolePoint(i, holeIndex);
                    holeBridgeIndex = i;

                }
            }
            sortedHoleData.push_back(HoleData{holeIndex, holeBridgeIndex, holeBridgePoint});
        }
        // Sort hole data so that holes furthest to the right are first
        std::sort(sortedHoleData.begin(), sortedHoleData.end(), [](const HoleData& x, const HoleData& y){
            return (x.bridgePoint.x < y.bridgePoint.x);
        });

        for (const HoleData& holeData : sortedHoleData)
        {
            // Find first edge which intersects with rightwards ray originating at the hole bridge point.
            v2_t rayIntersectPoint(std::numeric_limits<float>::max(), holeData.bridgePoint.y);
            std::list<Vertex> hullNodesPotentiallyInBridgeTriangle;
            std::list<Vertex>::iterator initialBridgeNodeOnHull;
            std::list<Vertex>::iterator currentNode = vertexList.begin();
            while (currentNode != vertexList.cend())
            {
                auto nextNode = currentNode;
                nextNode++;
                if (nextNode == vertexList.end())
                    nextNode = vertexList.begin();

                v2_t p0 = currentNode->position;
                v2_t p1 = nextNode->position;

                // at least one point must be to right of holeData.bridgePoint for intersection with ray to be possible
                if (p0.x > holeData.bridgePoint.x || p1.x > holeData.bridgePoint.x)
                {
                    // one point is above, one point is below
                    if (p0.y > holeData.bridgePoint.y != p1.y > holeData.bridgePoint.y)
                    {
                        float rayIntersectX = p1.x; // only true if line p0,p1 is vertical
                        if (!math::is_equals_almost(p0.x, p1.x))
                        {
                            float intersectY = holeData.bridgePoint.y;
                            float gradient = (p0.y - p1.y) / (p0.x - p1.x);
                            float c = p1.y - gradient * p1.x;
                            rayIntersectX = (intersectY - c) / gradient;
                        }

                        // intersection must be to right of bridge point
                        if (rayIntersectX > holeData.bridgePoint.x)
                        {
                            auto potentialNewBridgeNode = (p0.x > p1.x) ? currentNode : nextNode;
                            // if two intersections occur at same x position this means is duplicate edge
                            // duplicate edges occur where a hole has been joined to the outer polygon
                            bool isDuplicateEdge = math::is_equals_almost(rayIntersectX, rayIntersectPoint.x);

                            // connect to duplicate edge (the one that leads away from the other, already connected hole, and back to the original hull) if the
                            // current hole's bridge point is higher up than the bridge point of the other hole (so that the new bridge connection doesn't intersect).
                            auto prevPotentialNewBridgeNode = --potentialNewBridgeNode;
                            bool connectToThisDuplicateEdge = holeData.bridgePoint.y > prevPotentialNewBridgeNode->position.y;

                            if (!isDuplicateEdge || connectToThisDuplicateEdge)
                            {
                                // if this is the closest ray intersection thus far, set bridge hull node to point in line having greater x pos (since def to right of hole).
                                if (rayIntersectX < rayIntersectPoint.x || isDuplicateEdge)
                                {
                                    rayIntersectPoint.x = rayIntersectX;
                                    initialBridgeNodeOnHull = potentialNewBridgeNode;
                                }
                            }
                        }
                    }
                }

                // Determine if current node might lie inside the triangle formed by holeBridgePoint, rayIntersection, and bridgeNodeOnHull
                // We only need consider those which are reflex, since only these will be candidates for visibility from holeBridgePoint.
                // A list of these nodes is kept so that in next step it is not necessary to iterate over all nodes again.
                if (currentNode != initialBridgeNodeOnHull)
                {
                    if (!currentNode->IsConvex && p0.x > holeData.bridgePoint.x)
                    {
                        hullNodesPotentiallyInBridgeTriangle.push_back(*currentNode);
                    }
                }
                currentNode++;
            }

            // Check triangle formed by hullBridgePoint, rayIntersection, and bridgeNodeOnHull.
            // If this triangle contains any points, those points compete to become new bridgeNodeOnHull
            std::list<Vertex>::iterator validBridgeNodeOnHull = initialBridgeNodeOnHull;
            for (std::list<Vertex>::iterator nodePotentiallyInTriangle = hullNodesPotentiallyInBridgeTriangle.begin();
                nodePotentiallyInTriangle != hullNodesPotentiallyInBridgeTriangle.cend();
                nodePotentiallyInTriangle++)
            {
                if (nodePotentiallyInTriangle->index == initialBridgeNodeOnHull->index)
                    continue;

                // if there is a point inside triangle, this invalidates the current bridge node on hull.
                if (xpf::v2_t::is_point_in_triangle(holeData.bridgePoint, rayIntersectPoint, initialBridgeNodeOnHull->position, nodePotentiallyInTriangle->position))
                {
                    // Duplicate points occur at hole and hull bridge points.
                    bool isDuplicatePoint = validBridgeNodeOnHull->position == nodePotentiallyInTriangle->position;

                    // if multiple nodes inside triangle, we want to choose the one with smallest angle from holeBridgeNode.
                    // if is a duplicate point, then use the one occurring later in the list
                    float currentDstFromHoleBridgeY = std::abs(holeData.bridgePoint.y - validBridgeNodeOnHull->position.y);
                    float pointInTriDstFromHoleBridgeY = std::abs(holeData.bridgePoint.y - nodePotentiallyInTriangle->position.y);

                    if (pointInTriDstFromHoleBridgeY < currentDstFromHoleBridgeY || isDuplicatePoint)
                    {
                        validBridgeNodeOnHull = nodePotentiallyInTriangle;
                    }
                }
            }

            // Insert hole points (starting at holeBridgeNode) into vertex list at validBridgeNodeOnHull
            currentNode = validBridgeNodeOnHull;
            for (int i = holeData.bridgeIndex; i <= polygon.numPointsPerHole[holeData.holeIndex] + holeData.bridgeIndex; i++)
            {
                int previousIndex = currentNode->index;
                int currentIndex = polygon.IndexOfPointInHole(i % polygon.numPointsPerHole[holeData.holeIndex], holeData.holeIndex);
                int nextIndex = polygon.IndexOfPointInHole((i + 1) % polygon.numPointsPerHole[holeData.holeIndex], holeData.holeIndex);

                if (i == polygon.numPointsPerHole[holeData.holeIndex] + holeData.bridgeIndex) // have come back to starting point
                {
                    nextIndex = validBridgeNodeOnHull->index; // next point is back to the point on the hull
                }

                bool vertexIsConvex = IsConvex(polygon.points[previousIndex], polygon.points[currentIndex], polygon.points[nextIndex]);
                Vertex holeVertex{polygon.points[currentIndex], currentIndex, vertexIsConvex};
                currentNode = vertexList.insert(currentNode, holeVertex);
            }

            // Add duplicate hull bridge vert now that we've come all the way around. Also set its concavity
            auto currentNodeNextIter = currentNode; currentNodeNextIter++;
            v2_t nextVertexPos = (currentNodeNextIter == vertexList.end()) ? vertexList.front().position : currentNodeNextIter->position;
            bool isConvex = IsConvex(holeData.bridgePoint, validBridgeNodeOnHull->position, nextVertexPos);
            Vertex repeatStartHullVert{validBridgeNodeOnHull->position, validBridgeNodeOnHull->index, isConvex};
            vertexList.insert(currentNode, repeatStartHullVert);

            //Set concavity of initial hull bridge vert, since it may have changed now that it leads to hole vert
            auto prevvalidBridgeNodeOnHull = validBridgeNodeOnHull;
            prevvalidBridgeNodeOnHull--;

            auto nextvalidBridgeNodeOnHull = validBridgeNodeOnHull;
            nextvalidBridgeNodeOnHull++;

            const auto& nodeBeforeStartBridgeNodeOnHull = (prevvalidBridgeNodeOnHull== vertexList.cbegin()) ? vertexList.back() : *prevvalidBridgeNodeOnHull;
            const auto& nodeAfterStartBridgeNodeOnHull = (prevvalidBridgeNodeOnHull == vertexList.cend()) ? vertexList.front() : *nextvalidBridgeNodeOnHull;
            validBridgeNodeOnHull->IsConvex = IsConvex(nodeBeforeStartBridgeNodeOnHull.position, validBridgeNodeOnHull->position, nodeAfterStartBridgeNodeOnHull.position);
        }
        return vertexList;
    }

    // check if triangle contains any verts (note, only necessary to check reflex verts).
    bool TriangleContainsVertex(const Vertex& v0, const Vertex& v1, const Vertex& v2)
    {
        for (const auto& vertexNode : vertsInClippedPolygon)
        {
            if (!vertexNode.IsConvex) // convex verts will never be inside triangle
            {
                if (vertexNode.index != v0.index &&
                    vertexNode.index != v1.index &&
                    vertexNode.index != v2.index) // dont check verts that make up triangle
                {
                    if (PointInTriangle(v0.position, v1.position, v2.position, vertexNode.position))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    // v1 is considered a convex vertex if v0-v1-v2 are wound in a counter-clockwise order.
    static bool IsConvex(const v2_t& v0, const v2_t& v1, const v2_t& v2)
    {
        return SideOfLine(v0, v1, v2) == -1;
    }

    static int SideOfLine(v2_t a, v2_t b, v2_t c)
    {
        return (int)math::sign((c.x - a.x) * (-b.y + a.y) + (c.y - a.y) * (b.x - a.x));
    }

    static int SideOfLine(float ax, float ay, float bx, float by, float cx, float cy)
    {
        return (int)math::sign((cx - ax) * (-by + ay) + (cy - ay) * (bx - ax));
    }

    static bool PointInTriangle(v2_t a, v2_t b, v2_t c, v2_t p)
    {
        float area = 0.5f * (-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
        float s = 1 / (2 * area) * (a.y * c.x - a.x * c.y + (c.y - a.y) * p.x + (a.x - c.x) * p.y);
        float t = 1 / (2 * area) * (a.x * b.y - a.y * b.x + (a.y - b.y) * p.x + (b.x - a.x) * p.y);
        return s >= 0 && t >= 0 && (s + t) <= 1;

    }
};

} // xpf