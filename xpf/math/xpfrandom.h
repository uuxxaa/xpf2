#include <random>

namespace xpf {

struct random {
private:
    inline static std::uniform_int_distribution<uint32_t> s_uniformDist255 = std::uniform_int_distribution<uint32_t>(0, 255);
    inline static std::mt19937 m_rng;

public:
    static uint32_t random_255() { return s_uniformDist255(m_rng); }
};

} // xpf