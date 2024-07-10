#include <core/Rectangle.h>
#include <core/stringex.h>
#include <core/Thickness.h>

namespace xpf {

template rectangle<float> rectangle<float>::shrink(const Thickness& t) const;
template rectangle<int32_t> rectangle<int32_t>::shrink(const Thickness& t) const;
template rectangle<uint32_t> rectangle<uint32_t>::shrink(const Thickness& t) const;

template<typename T>
rectangle<T> rectangle<T>::shrink(const Thickness& t) const {
    return { T(x + t.l), T(y + t.t), T(w - t.l - t.r), T(h - t.t - t.b)};
}

template rectangle<float> rectangle<float>::expand(const Thickness& t) const;
template rectangle<int32_t> rectangle<int32_t>::expand(const Thickness& t) const;
template rectangle<uint32_t> rectangle<uint32_t>::expand(const Thickness& t) const;

template<typename T>
rectangle<T> rectangle<T>::expand(const Thickness& t) const {
    return { T(x - t.l), T(y - t.t), T(w + t.l + t.r), T(h + t.t + t.b)};
}

template std::string rectangle<float>::to_string(uint8_t precision) const;
template std::string rectangle<int32_t>::to_string(uint8_t precision) const;
template std::string rectangle<uint32_t>::to_string(uint8_t precision) const;

template<typename T> std::string rectangle<T>::to_string(uint8_t precision) const {
    return 
        "{x:" + xpf::stringex::to_string(x, precision) + 
        " y:" + xpf::stringex::to_string(y, precision) +
        " w:" + xpf::stringex::to_string(w, precision) +
        " h:" + xpf::stringex::to_string(h, precision) + "}";

}

} // xpf