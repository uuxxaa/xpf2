#import <Metal/Metal.h>
#include <string>
#include <vector>

#include <core/Types.h>

namespace xpf {

NSData* ToNS(const std::vector<byte_t>& bytes)
{
    return [NSData dataWithBytes:bytes.data() length:bytes.size()];
}

static NSString* ToNS(const char* charText, size_t cch)
{
    return [[NSString alloc] initWithBytes:charText length:cch encoding:NSUTF8StringEncoding];
}

NSString* ToNS(std::string_view str)
{
    return ToNS(str.data(), str.size());
}

NSURL* ToNSURL(std::string_view str)
{
    return [NSURL URLWithString: ToNS(str) ];
}

std::string to_string(const NSString* str)
{
    std::string buffer;

    if (str != nil)
    {
        NSInteger length = [str length];
        NSInteger sizeOfMaxBuffer = (length + 1);
        buffer.resize(length, 0);

        [str getCString:(char*)buffer.c_str() maxLength:sizeOfMaxBuffer encoding:NSUTF8StringEncoding];
    }

    return buffer;
}

} // xpf