#pragma once

#include <cstdint>

namespace durak {

// clang-format off
enum class DeckSize : std::uint8_t {
    Piquet      = 24,
    Skat        = 32,
    ThirtySix   = 36,
    Standard    = 52,
    WithJokers  = 54
};
// clang-format on

};