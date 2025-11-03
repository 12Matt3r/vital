#pragma once

#include <string>

namespace vital {

struct Version {
    static constexpr int major = 1;
    static constexpr int minor = 0;
    static constexpr int patch = 0;
    static constexpr int build = 0;
    
    static constexpr int audio_latency_ms = 5;
    static constexpr int max_voices = 32;
    static constexpr int max_effects = 8;
    static constexpr int max_oscillators = 4;
    
    static std::string getVersionString();
    static std::string getBuildInfo();
};

} // namespace vital