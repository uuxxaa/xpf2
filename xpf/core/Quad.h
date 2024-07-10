#pragma once
#include <math/v2_t.h>

namespace xpf {

struct Quad
{
    v2_t top_left;
    v2_t top_right;
    v2_t bottom_right;
    v2_t bottom_left;

    Quad() = default;
    Quad(
        v2_t top_left,
        v2_t top_right,
        v2_t bottom_right,
        v2_t bottom_left)
        : top_left(top_left)
        , top_right(top_right)
        , bottom_right(bottom_right)
        , bottom_left(bottom_left)
    {}

    Quad(Quad&&) = default;
    Quad(const Quad&) = default;
    Quad& operator=(Quad&&) = default;
    Quad& operator=(const Quad&) = default;

    Quad SliceFromLeft(float x) { // top left is zero position
        Quad left = *this;
        top_left.x = bottom_left.x = left.top_right.x = left.bottom_right.x = left.top_left.x + x;
        return left;
    }

    Quad SliceFromRight(float x) { // top right is zero position
        Quad left = *this;
        top_right.x = bottom_right.x = left.top_left.x = left.bottom_left.x = left.top_right.x - x;
        return left;
    }

    Quad SliceFromTop(float y) { // top left is zero position
        Quad topQuad = *this;
        top_left.y = top_right.y = topQuad.bottom_left.y = topQuad.bottom_right.y = topQuad.top_left.y + y;
        return topQuad;
    }

    Quad SliceFromBottom(float y) { // bottom Right is zero position
        Quad bottomQuad = *this;
        bottom_left.y = bottom_right.y = bottomQuad.top_left.y = bottomQuad.top_right.y = bottomQuad.bottom_right.y - y;
        return bottomQuad;
    }

    Quad ExtrudeFromLeft(float x) { // extrudes another Quad to the left
        Quad Quad = *this;
        Quad.top_left.x -= x;
        Quad.bottom_left.x -= x;
        Quad.top_right.x = Quad.top_left.x + x;
        Quad.bottom_left.x = Quad.bottom_left.x + x;
        return Quad;
    }
};

} // PCPP