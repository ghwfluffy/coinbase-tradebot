#include <gtb/Uuid.h>

#include <string>
#include <atomic>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>

using namespace gtb;

namespace
{

uint16_t getRand16()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint16_t> dist(0, UINT16_MAX);
    return dist(gen);
}

}

// b17c0112-dead-4e5d-abff-90865d1e13b1
std::string Uuid::generate()
{
    // Static header
    constexpr const uint32_t header = 0xb17c0112;

    // Monotomic counter
    static std::atomic<uint16_t> counter(1);
    const uint16_t current = counter++;

    // Random data
    const uint16_t random = getRand16();

    // Current time
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch();
    const uint64_t microseconds = static_cast<uint64_t>(epoch.count());

    // Assemble UUID parts into a string with dashes
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    // Header (4 bytes)
    oss << std::setw(8) << header << "-";

    // Counter (2 bytes)
    oss << std::setw(4) << current << "-";

    // Random (2 bytes)
    oss << std::setw(4) << random << "-";

    // Time (8 bytes, split into 2-bytes and 6-bytes)
    oss << std::setw(4) << static_cast<uint16_t>((microseconds >> 48) & 0xFFFFU) << "-";
    oss << std::setw(12) << (microseconds & 0xFFFFFFFFFFFFULL);

    // Return the UUID string
    return oss.str();
}
