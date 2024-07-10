#pragma once
#include <vector>
#include <stdint.h>
#include <stdexcept>

typedef uint8_t byte_t;

namespace xpf {
class BinaryReader
{
protected:
    const std::vector<byte_t>& m_data;
    const byte_t* m_pch;
    const byte_t* m_pend;

public:
    BinaryReader(const std::vector<byte_t>& data)
        : m_data(data)
        , m_pch(data.data())
        , m_pend(data.data() + data.size())
    {}

    void Skip(uint32_t bytes) { m_pch += bytes; }
    void SeekTo(uint32_t offset) { m_pch = m_data.data() + offset; }

    byte_t ReadByte()
    {
        if (m_pch == m_pend)
            throw std::exception();
        byte_t b = *m_pch;
        m_pch++;
        return b;
    }

    uint16_t ReadUInt16()
    {
        return ((uint16_t)ReadByte()) << 8 | ReadByte();
    }

    uint32_t ReadUInt32()
    {
        return ((uint32_t)ReadByte()) << 24 | ((uint32_t)ReadByte()) << 16 | ((uint32_t)ReadByte()) << 8 | ReadByte();
    }
};

} // xpf
