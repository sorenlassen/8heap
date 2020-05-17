#pragma once

#include <cstdint>
#include <array>
#include <limits>

// 48 bit unsigned integer with conversions to and from uin64_t
struct U48 {
  static constexpr uint64_t k1 = uint64_t(1) << 16, k2 = uint64_t(1) << 32;
  std::array<uint16_t, 3> u48;
  constexpr U48(uint64_t u) : u48{uint16_t(u), uint16_t(u / k1), uint16_t(u / k2)} {}
  operator uint64_t() const { return u48[0] + k1 * u48[1] + k2 * u48[2]; }
};

namespace std {
  template<> class numeric_limits<U48> {
   public:
    static constexpr U48 max() { return (uint64_t(1) << 48) - 1; }
  };
}
