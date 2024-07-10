#pragma once
#include <core/stringex.h>
#include <core/Log.h>
#include <core/Types.h>

#include <array>
#include <functional>
#include <fstream>
#include <sstream>
#include <vector>

namespace xpf {

class FileSystem {

private:
    inline static std::function<std::vector<byte_t>(xpf::string_viewex)> m_binary_callback;
    inline static std::function<std::string(xpf::string_viewex)> m_text_callback;

public:
    static xpf::string_viewex GetPath(xpf::string_viewex str) {
        auto pos = str.find_last_of("/\\");
        if (pos == xpf::string_viewex::npos) return "";

        return str.substr(0, pos);
    }

    static xpf::string_viewex GetFilename(xpf::string_viewex str) {
        auto pos = str.find_last_of("/\\");
        if (pos == xpf::string_viewex::npos) return str;

        return str.substr(pos + 1);
    }

    static xpf::string_viewex GetFileExtension(xpf::string_viewex str) {
        auto pos = str.find_last_of(".");
        if (pos == xpf::string_viewex::npos) return "";

        return str.substr(pos);
    }

    static void SetFileLoadCallback(std::function<std::vector<byte_t>(xpf::string_viewex)>&& fn) {
        m_binary_callback = std::move(fn);
    }
    static void SetFileLoadCallback(std::function<std::string(xpf::string_viewex)>&& fn) {
        m_text_callback = std::move(fn);
    }

    static std::vector<byte_t> LoadFile(xpf::string_viewex filename) {
        // Log::info("Loading: " + filename);
        if (m_binary_callback != nullptr)
            return m_binary_callback(filename);

        constexpr size_t c_readBufferSize = 4096;
        std::ifstream stream(filename.data(), std::ios::binary | std::ios::in);
        stream.exceptions(std::ios_base::badbit);

        if (!stream.is_open())
            return {};

        std::vector<byte_t> result;
        std::array<byte_t, c_readBufferSize> buffer;
        while (stream.read(reinterpret_cast<char*>(buffer.data()), c_readBufferSize))
        {
            result.insert(result.end(), std::begin(buffer), std::begin(buffer) + stream.gcount());
        }

        result.insert(result.end(), std::begin(buffer), std::begin(buffer) + stream.gcount());
        Log::info("Loaded: " + filename);

        return result;
    }

    static std::string LoadTextFiles(std::initializer_list<std::string_view> files, bool useCallback = true) {
        std::stringstream sstr;

        for (const auto& filename : files) {
            Log::info("Loading: " + filename);
            if (m_text_callback != nullptr && useCallback) {
                sstr << m_text_callback(filename);
            } else {
                std::ifstream stream(filename.data(), std::ios::in);
                if (!stream.is_open())
                    throw std::runtime_error("Failed to open: " + filename);

                Log::info("Loaded: " + filename);
                sstr << stream.rdbuf();
                stream.close();
            }
        }

        return sstr.str();
    }

    static std::string LoadTextFile(std::string_view filename, bool useCallback = true) {
        Log::info("Loading: " + filename);
        if (m_text_callback != nullptr && useCallback)
            return m_text_callback(filename);

        std::stringstream sstr;
        std::ifstream stream(filename.data(), std::ios::in);
        if (!stream.is_open())
            throw std::runtime_error("Failed to open: " + filename);

        sstr << stream.rdbuf();
        stream.close();
        Log::info("Loaded: " + filename);

        return sstr.str();
    }

    static std::pair<bool, xpf::stringex> LoadTextFileNoThrow(std::string_view filename) {
        Log::info("Loading: " + filename);

        std::stringstream sstr;
        std::ifstream stream(filename.data(), std::ios::in);
        if (!stream.is_open()) {
            Log::info("Failed to open: " + filename);
            return {false, "Failed to open: " + filename };
        }

        sstr << stream.rdbuf();
        stream.close();
        Log::info("Loaded: " + filename);

        return {true, sstr.str()};
    }

    static bool SaveFile(std::string_view filename, std::string_view str) {
        return SaveFile(filename, reinterpret_cast<const byte_t*>(str.data()), str.size());
    }

    static bool SaveFile(std::string_view filename, const byte_t* pdata, size_t size) {
        std::ofstream stream(filename.data(), std::ios::out | std::ios::binary);
        stream.exceptions(std::ios_base::badbit);

        if (!stream)
            return false;

        stream.write((char*)(pdata), size);
        stream.close();
        return true;
    }

#if 0
    enum class FileType {
        executable,
        binary,
        image,
        text,
        font,
    };

    static file_type GetFileType(xpf::string_viewex filename) {
        xpf::string_viewex ext = GetFileExtension(filename).to_lower();
        if (ext == ".exe" || ext == ".dll")
            return file_type::executable;
        if (ext == ".ttf")
            return file_type::font;
        if (ext == ".gif" || ext == ".jpg" || ext == ".png" || ext == ".ico")
            return file_type::image;
        return file_type::text;
    }

    static bool file_exists(std::string_view filename) {
        if (!std::filesystem::exists(filename)) {
            log::info("Missing file: " + filename);
            return false;
        }
        return true;
    }
#endif
};

} // xpf