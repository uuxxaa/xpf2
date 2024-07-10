#pragma once
#include <iostream>
#include <string>
#include <vector>

namespace xpf {

class Log {
public:
    static inline bool report_to_cout = true;

private:
    static inline std::vector<std::string> s_log;

public:
    static void Assert(bool condition, std::string_view str) { if (!condition) error(str); }
    static void info(std::string_view str) { s_log.emplace_back(str); if (report_to_cout) std::cout << "info: " << str << std::endl; }
    static void error(std::string_view str) { s_log.emplace_back(str); if (report_to_cout) std::cout << "error: " << str << std::endl; }
};

} // xpf