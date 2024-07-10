#pragma once
#include <stdint.h>
#include <core/stringex.h>

namespace xpf {

struct Hash_64_Constants {
    typedef uint64_t storage;
    constexpr static const uint64_t basis = 14695981039346656037ULL;
    constexpr static const uint64_t prime = 1099511628211ULL;
};

struct Hash_32_Constants {
    typedef uint32_t storage;
    constexpr static const uint64_t basis = 2166136261U;
    constexpr static const uint64_t prime = 16777619U;
};

template <typename THash>
class Hash
{
protected:
    typename THash::storage m_hash = THash::basis;

public:
    Hash() = default;

    template <typename T>
    Hash& append(const T* item, size_t length) {
        while (length--) {
            append(static_cast<typename THash::storage>(*item));
            item++;
        }

        return *this;
    }

    Hash& Append(const typename THash::storage& item) {
        m_hash ^= item;
        m_hash *= THash::prime;
        return *this;
    }

    Hash& Append(std::string_view str) {
        return append(str.cbegin(), str.cend());
    }

    template <typename T> Hash& AppendCollection(const T& collection) {
        return Append(collection.cbegin(), collection.cend());
    }

    template <typename T> Hash& Append(const T& start, const T end) {
        for (auto it = start; it != end; ++it) {
            Append(static_cast<typename THash::storage>(*it));
        }
        return *this;
    }

    auto Finalize() {
        typename THash::storage result = m_hash;
        m_hash = THash::basis;

        return result;
    }
};

typedef Hash<Hash_64_Constants> hash64;
typedef Hash<Hash_32_Constants> hash32;

constexpr size_t ConstexprHash(const char* input) {
    size_t Hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
    const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

    while (*input) {
        Hash ^= static_cast<size_t>(*input);
        Hash *= prime;
        ++input;
    }

    return Hash;
}

} // xpf
