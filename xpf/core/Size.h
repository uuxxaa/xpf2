#pragma once
#include <stdint.h>

namespace xpf {

template<typename T>
struct Size
{
    T width = 0;
    T height = 0;

    Size& operator*(T scale) { width *= scale; height *= scale; return *this; }
};

typedef Size<float> SizeF;
typedef Size<int32_t> SizeI;
typedef Size<uint32_t> SizeUI;

} // xpf
