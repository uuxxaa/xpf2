#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

namespace xpf {

enum class string_comparison {
    ordinal,
    ordinal_ignoreCase
};

enum class string_split_options {
    none,
    remove_empty,
};

class stringex;

class string_viewex : public std::string_view {
public:
    string_viewex();
    string_viewex(std::string_view view);
    string_viewex(const std::string& str);
    string_viewex(const char* psz);
    string_viewex(const char* psz, size_t len);

    stringex to_lower() const;
    uint32_t get_hashcode(string_comparison comparison = string_comparison::ordinal) const;
    int32_t compare(std::string_view value, string_comparison comparison = string_comparison::ordinal) const;
    bool equals(std::string_view value, string_comparison comparison = string_comparison::ordinal) const;
    bool starts_with(std::string_view starting, string_comparison comparison = string_comparison::ordinal) const;
    bool ends_with(std::string_view ending, string_comparison comparison = string_comparison::ordinal) const;
    bool contains(std::string_view lookup) const;
    std::vector<xpf::stringex> split(std::string_view delimiter) const;
    std::vector<xpf::stringex> split(char delimiter) const;
    void split(char delimiter, string_split_options options, std::function<void(xpf::string_viewex)>&& fn) const;
    xpf::string_viewex trim() const;
    bool find_match(
        xpf::string_viewex lookup,
        /*out*/ xpf::string_viewex& strBeforeMatch,
        /*out*/ xpf::string_viewex& strMatch,
        string_comparison comparison);
};

class stringex : public std::string {
public:
    stringex();
    stringex(std::string_view view);
    stringex(std::string&& str);
    stringex(const char* str);
    stringex(const char* start, const char* end);
    stringex(uint32_t count, char ch) : std::string(count, ch) { }

    stringex to_lower();

    static char32_t utf8_to_utf32(std::string_view text, size_t& i);
    static uint32_t get_char_count(std::string_view text);

    void append32(char32_t ch32);

    stringex& appendex(float t, int precision = 3);
    stringex& appendex(int32_t t);
    stringex& appendex(uint32_t t);
    stringex& appendex(std::string_view v);
    stringex& newline();

    static stringex to_string(float value, uint8_t precision);
    static stringex to_string_with_digits(int32_t value, uint8_t digits);
    static stringex to_hex(uint32_t value, uint8_t digits);
    int32_t compare(std::string_view value, string_comparison comparison = string_comparison::ordinal) const;
    bool equals(std::string_view value, string_comparison comparison = string_comparison::ordinal) const;
    bool starts_with(std::string_view starting, string_comparison comparison = string_comparison::ordinal) const;
    bool ends_with(std::string_view ending, string_comparison comparison = string_comparison::ordinal) const ;
    bool contains(std::string_view lookup) const;
    std::vector<xpf::stringex> split(std::string_view delimiter) const;
    std::vector<xpf::stringex> split(char delimiter) const;

    static xpf::stringex join(const std::vector<xpf::string_viewex>& strs, xpf::string_viewex delimeter);
    static xpf::stringex join(const std::vector<xpf::stringex>& strs, xpf::string_viewex delimeter);
};

inline std::string operator+(const char* str1, std::string_view str2) {
    return std::string(str1).append(str2);
}

inline std::string operator+(std::string_view str1, const char* str2) {
    return std::string(str1).append(str2);
}

inline std::string operator+(std::string_view str1, std::string_view str2) {
    return std::string(str1).append(str2);
}

struct stringview_map_helper_ignorecase {
    size_t operator()(xpf::string_viewex key) const { return key.get_hashcode(string_comparison::ordinal_ignoreCase); }
    bool operator()(xpf::string_viewex key1, xpf::string_viewex key2) const { return key1.equals(key2, string_comparison::ordinal_ignoreCase); }
};

struct stringview_map_helper {
    size_t operator()(xpf::string_viewex key) const { return key.get_hashcode(); }
    bool operator()(xpf::string_viewex key1, xpf::string_viewex key2) const { return key1.equals(key2); }
};

template <typename TValueType>
using map_stringview_ignorecase = std::unordered_map<xpf::string_viewex, TValueType, stringview_map_helper_ignorecase, stringview_map_helper_ignorecase>;

template <typename TValueType>
using map_stringview = std::unordered_map<xpf::string_viewex, TValueType, stringview_map_helper, stringview_map_helper>;

} // xpf
