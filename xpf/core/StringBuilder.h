#pragma once
#include <vector>
#include <core/stringex.h>

namespace xpf {

class StringBuilder {
protected:
    std::vector<xpf::stringex> m_storage;
    std::vector<xpf::string_viewex> m_strs;
    size_t m_length = 0;

public:
    StringBuilder() = default;

    StringBuilder& AppendView(xpf::string_viewex str)
    {
        m_strs.push_back(str);
        m_length += str.length();
        return *this;
    }

    StringBuilder& AppendNewline()
    {
        m_strs.push_back("\n");
        m_length++;
        return *this;
    }

    StringBuilder& Append(xpf::stringex&& str)
    {
        m_storage.emplace_back(std::move(str));
        m_strs.push_back(m_storage.back());
        m_length += str.length();
        return *this;
    }

    xpf::stringex ToString()
    {
        xpf::stringex str;
        str.reserve(m_length);
        for (const auto& s : m_strs) {
            str.append(s);
        }

        return str;
    }
};

} // xpf