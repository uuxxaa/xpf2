#pragma once
#include <string>
#include <vector>
#include <stdint.h>
typedef uint8_t byte_t;

namespace xpf::resources {
std::string_view opengl_shader_vert();
std::string_view opengl_shader_frag();
std::string_view xpf_metal();
} // xpf::resources
