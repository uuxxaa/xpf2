#include "stringex.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
//#include "hash.h"

namespace xpf {

#pragma region helpers
struct not_space { bool operator()(char c) const { return !std::isspace(static_cast<unsigned char>(c)); } };

static bool string_viewex_equals_ch(char ch1, char ch2) {
    return ch1 == ch2;
}

static bool string_viewex_equals_chi(char ch1, char ch2) {
    return std::tolower(ch1) == std::tolower(ch2);
}
#pragma endregion

#pragma region xpf::string_viewex
string_viewex::string_viewex() = default;
string_viewex::string_viewex(std::string_view view) : std::string_view(view) {}
string_viewex::string_viewex(const std::string& str) : std::string_view(str) {}
string_viewex::string_viewex(const char* psz) : std::string_view(psz) {}
string_viewex::string_viewex(const char* psz, size_t len) : std::string_view(psz, len) {}

stringex string_viewex::to_lower() const {
    stringex result(*this);
    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char c){ return std::tolower(c); });

    return result;
}

// uint32_t string_viewex::get_hashcode(string_comparison comparison) const {
//     hash32 hash;
//     if (comparison == string_comparison::ordinal) {
//         for (char ch : *this) {
//             hash.append(ch);
//         }
//     } else {
//         for (char ch : *this) {
//             hash.append(std::tolower(ch));
//         }
//     }
// 
//     return hash.finalize();
// }

int32_t string_viewex::compare(std::string_view value, string_comparison comparison) const {
    if (comparison == string_comparison::ordinal)
        return std::string_view::compare(value);

    // string_comparison::ordinal_ignoreCase
    size_t alen = length();
    size_t blen = value.length();
    size_t len = std::min(alen, blen);
    int diff;
    for (size_t i = 0; i < len; i++) {
        diff = std::tolower(this->at(i)) - std::tolower(value[i]);
        if (diff != 0)
            return diff < 0 ? -1 : 1;
    }

    if (alen < blen) return -1;
    if (alen == blen) return 0;
    return 1;
}

bool string_viewex::equals(std::string_view value, string_comparison comparison) const {
    return compare(value, comparison) == 0;
}

bool string_viewex::starts_with(std::string_view starting, string_comparison comparison) const {
    if (length() < starting.length())
        return false;

    return string_viewex(data(), starting.length()).equals(starting, comparison);
}

bool string_viewex::ends_with(std::string_view ending, string_comparison comparison) const {
    if (length() < ending.length())
        return false;

    return string_viewex(data() + length() -  ending.length()).equals(ending, comparison);
}

bool string_viewex::contains(std::string_view lookup) const { return find(lookup) != std::string::npos; }


std::vector<xpf::stringex> string_viewex::split(std::string_view delimiter) const {
    std::vector<xpf::stringex> result;
    xpf::stringex s = *this;
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        xpf::stringex token = s.substr(0, pos);
        result.push_back(std::move(token));
        s.erase(0, pos + delimiter.length());
    }

    if (!s.empty())
        result.push_back(s);

    return result;
}

std::vector<xpf::stringex> string_viewex::split(char delimiter) const {
    std::vector<xpf::stringex> result;
    xpf::stringex s = *this;
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        xpf::stringex token = s.substr(0, pos);
        result.push_back(std::move(token));
        s.erase(0, pos + 1);
    }

    if (!s.empty())
        result.push_back(s);

    return result;
}

void string_viewex::split(char delimiter, string_split_options options, std::function<void(xpf::string_viewex)>&& fn) const {
    size_t pos = 0;
    size_t start = 0;
    while ((pos = find(delimiter, start)) != std::string::npos) {
        if (options == string_split_options::none || pos - start != 0) {
            xpf::string_viewex token = substr(start, pos - start);
            fn(token);
        }
        start = pos + 1;
    }

    if (start < length())
        fn(substr(start));
}

xpf::string_viewex string_viewex::trim() const {
    const auto content_start = std::find_if(cbegin(), cend(), not_space());
    if (content_start == cend())
        return std::string_view();

    const auto content_end = std::find_if(crbegin(), crend(), not_space());
    const size_t prefix_len = content_start - cbegin();
    const size_t content_len = content_end.base() - content_start;
    return substr(prefix_len, content_len);
}

bool string_viewex::find_match(
    xpf::string_viewex lookup,
    /*out*/ xpf::string_viewex& strBeforeMatch,
    /*out*/ xpf::string_viewex& strMatch,
    string_comparison comparison)
{
    strBeforeMatch = strMatch = "";
    size_t lookup_size = lookup.size();
    size_t size = this->size();
    if (lookup_size > size)
        return false;

    auto equalsfn = comparison == string_comparison::ordinal
        ? string_viewex_equals_ch
        : string_viewex_equals_chi;

    size_t startIndex = 0;
    bool found = false;
    for (size_t i = 0; i < size; i++) {
        size_t j = 0;
        startIndex = i;
        for (;j < lookup_size; j++) {
            if (!equalsfn(at(i), lookup.at(j))) {
                found = false;
                break;
            }
        }

        if (j == lookup_size)
            break;
    }

    if (found)
    {
        strBeforeMatch = substr(0, startIndex);
        strMatch = substr(startIndex, lookup_size);
    }

    return found;
}
#pragma endregion xpf::string_viewex

#pragma region xpf::stringex
stringex::stringex() = default;
stringex::stringex(std::string_view view) : std::string(view) {}
stringex::stringex(std::string&& str) : std::string(std::move(str)) {}
stringex::stringex(const char* str) : std::string(str) {}
stringex::stringex(const char* start, const char* end) : std::string(start, end) {}

stringex stringex::to_lower() {
    std::string result = *this;
    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char c){ return std::tolower(c); });

    return result;
}

/*static*/ char32_t stringex::utf8_to_utf32(std::string_view text, size_t& i) {
    char32_t ch = 0;

    if (i < text.size() && (text[i] & 0b10000000) == 0) {
        // 1 byte code point, ASCII
        ch = text[i];
        i += 1;
    }
    else if (i + 1 < text.size() && (text[i] & 0b11100000) == 0b11000000) {
        // 2 byte code point
        ch = (uint32_t(text[i]) & 0b00011111) << 6 | (uint32_t(text[i + 1]) & 0b00111111);
        i += 2;
    }
    else if (i + 2 < text.size() && (text[i] & 0b11110000) == 0b11100000) {
        // 3 byte code point
        ch = (uint32_t(text[i]) & 0b00001111) << 12 | (uint32_t(text[i + 1]) & 0b00111111) << 6 | (uint32_t(text[i + 2]) & 0b00111111);
        i += 3;
    }
    else if (i + 3 < text.size()) {
        // 4 byte code point
        ch = (uint32_t(text[i]) & 0b00000111) << 18 | (uint32_t(text[i + 1]) & 0b00111111) << 12 | (uint32_t(text[i + 2]) & 0b00111111) << 6 | (uint32_t(text[i + 3]) & 0b00111111);
        i += 4;
    }

    return ch;
}

/*static*/ uint32_t stringex::get_char_count(std::string_view text) {
    size_t i = 0;
    size_t txtlen = text.length();
    uint32_t count = 0;
    while (i < txtlen) {
        const uint32_t ch = stringex::utf8_to_utf32(text, i);
        if (ch == 0) [[unlikely]]
            break;
        count++;
    }

    return count;
}

void stringex::append32(char32_t ch32) {
    if (ch32 <= 0x7F) {
        append(1, char(ch32));
    }
    else if (ch32 <= 0x7FF) {
        append(1, char(0xC0 | (ch32 >> 6)));           /* 110xxxxx */
        append(1, char(0x80 | (ch32 & 0x3F)));         /* 10xxxxxx */
    }
    else if (ch32 <= 0xFFFF) {
        append(1, char(0xE0 | (ch32 >> 12)));          /* 1110xxxx */
        append(1, char(0x80 | ((ch32 >> 6) & 0x3F)));  /* 10xxxxxx */
        append(1, char(0x80 | (ch32 & 0x3F)));         /* 10xxxxxx */
    }
    else if (ch32 <= 0x10FFFF) {
        append(1, char(0xF0 | (ch32 >> 18)));          /* 11110xxx */
        append(1, char(0x80 | ((ch32 >> 12) & 0x3F))); /* 10xxxxxx */
        append(1, char(0x80 | ((ch32 >> 6) & 0x3F)));  /* 10xxxxxx */
        append(1, char(0x80 | (ch32 & 0x3F)));         /* 10xxxxxx */
    }
}

stringex& stringex::appendex(float t, int precision) { std::string::append(to_string(t, precision)); return *this; }
stringex& stringex::appendex(int32_t t) { std::string::append(std::to_string(t)); return *this; }
stringex& stringex::appendex(uint32_t t) { std::string::append(std::to_string(t)); return *this; }
stringex& stringex::appendex(std::string_view v) { std::string::append(v); return *this; }
stringex& stringex::newline() { append("\n"); return *this; }

/*static*/ stringex stringex::to_string(float value, uint8_t precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

/*static*/ stringex stringex::to_string_with_digits(int32_t value, uint8_t digits) {
    std::stringstream stream;
    stream << std::setw(digits) << std::setfill('0') << value;
    return stream.str();
}

/*static*/ stringex stringex::to_hex(uint32_t value, uint8_t digits) {
    std::stringstream stream;
    stream << std::setw(digits) << std::hex << value;
    return stream.str();
}

int32_t stringex::compare(std::string_view value, string_comparison comparison) const {
    return string_viewex(*this).compare(value, comparison);
}

bool stringex::equals(std::string_view value, string_comparison comparison) const {
    return string_viewex(*this).equals(value, comparison);
}

bool stringex::starts_with(std::string_view starting, string_comparison comparison) const {
    return string_viewex(*this).starts_with(starting, comparison);
}

bool stringex::ends_with(std::string_view ending, string_comparison comparison) const {
    return string_viewex(*this).ends_with(ending, comparison);
}

bool stringex::contains(std::string_view lookup) const { return find(lookup) != std::string::npos; }

std::vector<xpf::stringex> stringex::split(std::string_view delimiter) const {
    std::vector<xpf::stringex> result;
    xpf::stringex s = *this;
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        xpf::stringex token = s.substr(0, pos);
        result.push_back(std::move(token));
        s.erase(0, pos + delimiter.length());
    }

    if (!s.empty())
        result.push_back(s);

    return result;
}

std::vector<xpf::stringex> stringex::split(char delimiter) const {
    std::vector<xpf::stringex> result;
    xpf::stringex s = *this;
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        xpf::stringex token = s.substr(0, pos);
        result.push_back(std::move(token));
        s.erase(0, pos + 1);
    }

    if (!s.empty())
        result.push_back(s);

    return result;
}

/*static*/ xpf::stringex stringex::join(const std::vector<xpf::string_viewex>& strs, xpf::string_viewex delimeter) {
    xpf::stringex result;
    for (const auto& str : strs) {
        if (!result.empty())
            result.append(delimeter);
        result.append(str);
    }

    return result;
}

/*static*/ xpf::stringex stringex::join(const std::vector<xpf::stringex>& strs, xpf::string_viewex delimeter) {
    xpf::stringex result;
    for (const auto& str : strs) {
        if (!result.empty())
            result.append(delimeter);
        result.append(str);
    }

    return result;
}
#pragma endregion

} // xpf
