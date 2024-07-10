#pragma once
#include <string>
#include <core/Types.h>

namespace xpf {

NSData* ToNS(const std::vector<byte_t>& bytes);

NSString* ToNS(std::string_view);
NSURL* ToNSURL(std::string_view);
std::string to_string(const NSString* str);

} // xpf