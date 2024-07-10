#pragma once
#include <vector>
#include <math/v2_t.h>

namespace xpf {

class Polygon
{
public:
    std::vector<v2_t> points;
    size_t numPoints = 0;
    size_t numHullPoints;
    std::vector<size_t> numPointsPerHole;
    size_t numHoles;
    std::vector<size_t> holeStartIndices;

    Polygon(const std::vector<v2_t>& hull)
        : Polygon(hull, {})
    {
    }

    Polygon(const std::vector<v2_t>& hull, const std::vector<std::vector<v2_t>>& holes) {
        numHullPoints = hull.size();
        numHoles = holes.size();

        numPointsPerHole.resize(numHoles);
        holeStartIndices.resize(numHoles);
        size_t numHolePointsSum = 0;

        for (size_t i = 0; i < holes.size(); i++) {
            numPointsPerHole[i] = holes.size();
            holeStartIndices[i] = numHullPoints + numHolePointsSum;
            numHolePointsSum = holeStartIndices[i];
        }

        numPoints = numHullPoints + numHolePointsSum;
        points.resize(numPoints);

        // add hull points - ensure ccw order
        bool reverseHullPointsOrder = !PointsAreCounterClosewise(hull);
        for (size_t i = 0; i < numHullPoints; i++)
        {
            points[i] = hull[reverseHullPointsOrder ? numHullPoints - 1 - i:  i];
        }

        for (size_t i = 0; i < numHoles; i++)
        {
            bool reverseHolePointsOrder = !PointsAreCounterClosewise(holes[i]);
            for (size_t j = 0; j < holes[i].size(); j++)
            {
                points[IndexOfPointInHole(i, j)] = holes[i][reverseHullPointsOrder ? holes[i].size() - j - 1:  j];
            }
        }
    }

    bool PointsAreCounterClosewise(const std::vector<v2_t>& testPoints) const
    {
            float signedArea = 0;
            for (size_t i = 0; i < testPoints.size(); i++)
            {
                size_t nextIndex = (i + 1) % testPoints.size();
                signedArea += (testPoints[nextIndex].x - testPoints[i].x) * (testPoints[nextIndex].y + testPoints[i].y);
            }

            return signedArea < 0;
    }

    size_t IndexOfFirstPointInHole(size_t holeIndex) const
    {
        return holeStartIndices[holeIndex];
    }

    size_t IndexOfPointInHole(size_t index, size_t holeIndex) const
    {
        return holeStartIndices[holeIndex] + index;
    }

    v2_t GetHolePoint(size_t index, size_t holeIndex) const
    {
        return points[holeStartIndices[holeIndex] + index];
    }
};

}