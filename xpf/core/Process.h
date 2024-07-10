#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <cstring> // sterror
#include <cstdio>  // popen, FILE, fgets, fputs
#include <cassert>
#include <functional>
#include <core/stringex.h>

#if defined(PLATFORM_WINDOWS)
#define popen _popen
#define pclose _pclose
#endif

namespace xpf {

// How to use it
//
// std::vector<std::string> lines;
// xpf::Process::Run("git status", [&](bool succees, const std::string& line) {
//     lines.push_back(line);
// });

class Process {
protected:
    FILE* m_file = nullptr;
    Process() = default;

public:
    ~Process() {
        if (m_file != nullptr)
            ::pclose(m_file);
    }

    static bool Run(const xpf::stringex& command, std::function<void(bool, xpf::stringex&&)>&& fn) {
        return Process().RunInner(command, std::move(fn));
    }

    static std::pair<bool, xpf::stringex> Run(const xpf::stringex& command) {
        return Process().RunInner(command);
    }

protected:
    std::pair<bool, xpf::stringex> RunInner(const xpf::stringex& command) {
        m_file = ::popen(command.c_str(), "r");

        int myErrno = errno;
        if (0 == m_file) {
            return {false, "failed to start Process"};
        }

        std::stringstream result;
        if (!ReadAll(result)) {
            ::pclose(m_file);
            return {false, "failed to Run command"};
        }

        ::pclose(m_file);

        return {true, result.str()};
    }

    bool RunInner(const xpf::stringex& command, std::function<void(bool, xpf::stringex&&)> &&fn) {
        m_file = ::popen(command.c_str(), "r");

        int myErrno = errno;
        if (0 == m_file) {
            fn(false, "Process: Failed to start Process");
            return false;
        }

        std::stringstream result;
        if (!ReadAll(result)) {
            fn(false, "Process: Failed to Run command");
            return false;
        }

        for (xpf::stringex line; std::getline(result, line);) {
            if (line.ends_with("\r"))
                line.resize(line.length() - 1);
            fn(true, std::move(line));
        }

        return true;
    }

private:
    bool ReadAll(std::stringstream &result_stream) {
        const int BUFF_SIZE = 2 * 1024;
        if (0 == m_file)
            return false;

        // allocate working buff in auto var, fill with nulls
        for (;;) {
            char buff[BUFF_SIZE] = {0};
            char* stat = std::fgets(buff, BUFF_SIZE, m_file);
            if (feof(m_file))
                return true;

            if (ferror(m_file))
                return false;

            if (strlen(buff) != 0)
                result_stream << buff;
        }

        return false;
    }
};

} // xpf