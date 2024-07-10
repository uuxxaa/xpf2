#pragma
#include <math/v2_t.h>

namespace xpf::math {

// https://en.wikipedia.org/wiki/Heron's_formula
inline float triangle_area_helper(float a, float b, float c)
{
    float p = (a + b + c) / 2.0f;
    return sqrtf(p * (p - a) * (p - b) * (p - c));
}

inline float triangle_area(v2_t v1, v2_t v2, v2_t v3)
{
    return triangle_area_helper(
        (v1 - v2).magnitude(),
        (v2 - v3).magnitude(),
        (v3 - v1).magnitude());
}

//
//                e1
//          edge /
//              / <-- dx --> *
//             /             point
//            e0
//  
// calculation of distance from edge to point:
//  y(x) = e0.y + (e1.y - e0.y) / (e1.x - e0.x) * (x - e0.x)    - the edge line equation
//  x(y) = (y - e0.y) / (e1.y - e0.y) * (e1.x - e0.x) + e0.x    - inverse equation of line
//  dx = point.x - x(point.y)                                   - final equation for distance
inline float distance_from_edge(v2_t point, v2_t e0, v2_t e1) {
    float dy = e1.y - e0.y;
    if (fabs(dy) > xpf::math::Epsilon)
        return point.x - (point.y - e0.y) / dy * (e1.x - e0.x) - e0.x;

    // edge is horizontal (e0.y ~== e1.y)
    float dx = e1.x - e0.x;
    if (fabs(dy) > xpf::math::Epsilon)
        return point.y - (point.x - e0.x) / dx * (e1.y - e0.y) - e0.y;

    // edge is a point (e0 ~== e1) use point to point distance
    return v2_t::distance(e0, point);
}

// are 3 vertices given are left winding or not
inline bool isLeftWinding(v2_t a, v2_t b, v2_t c) {
  return (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x) > 0;
}

// which side of the a-c line lies the point b
inline int SideOfLine(v2_t a, v2_t b, v2_t c)
{
    return (int)math::sign((c.x - a.x) * (-b.y + a.y) + (c.y - a.y) * (b.x - a.x));
}

// https://stackoverflow.com/questions/34474336/decide-if-a-point-is-on-or-close-enough-to-a-line-segment
inline bool CheckIfPointIsCloseToLine(v2_t point, v2_t p0, v2_t p1, float maxDistance) {
    float xp = point.x, yp = point.y;
    float x1 = p0.x, y1 = p0.y, x2=p1.x, y2=p1.y;
    float dxL = x2 - x1, dyL = y2 - y1;  // line: vector from (x1,y1) to (x2,y2)
    float dxP = xp - x1, dyP = yp - y1;  // point: vector from (x1,y1) to (xp,yp)
    float dxQ = xp - x2, dyQ = yp - y2;  // extra: vector from (x2,y2) to (xp,yp)

    float squareLen = dxL * dxL + dyL * dyL;  // squared length of line
    float dotProd   = dxP * dxL + dyP * dyL;  // squared distance of point from (x1,y1) along line
    float crossProd = dyP * dxL - dxP * dyL;  // area of parallelogram defined by line and point

    // perpendicular distance of point from line
    float distance = std::abs(crossProd) / std::sqrtf(squareLen);

    // distance of (xp,yp) from (x1,y1) and (x2,y2)
    float distFromEnd1 = std::sqrtf(dxP * dxP + dyP * dyP);
    float distFromEnd2 = std::sqrtf(dxQ * dxQ + dyQ * dyQ);

    // if the point lies beyond the ends of the line, check if
    // it's within maxDistance of the closest end point
    if (dotProd < 0) return distFromEnd1 <= maxDistance;
    if (dotProd > squareLen) return distFromEnd2 <= maxDistance;

    // else check if it's within maxDistance of the line
    return distance <= maxDistance;
}

} // xpf::math