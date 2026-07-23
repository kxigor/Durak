#pragma once

#include <cstdint>
#include <include/durak/engine/config.hpp>

namespace durak::rules {

// clang-format off
enum class Suit : std::uint8_t {
  Clubs   ,
  Diamonds,
  Hearts  ,
  Spades
};

enum class Rank : std::uint8_t {
  Two   =  2,
  Three =  3,
  Four  =  4,
  Five  =  5,
  Six   =  6,
  Seven =  7,
  Eight =  8,
  Nine  =  9,
  Ten   = 10,

  Jack  = 11,
  Queen = 12,
  King  = 13,
  Ace   = 14,
  /*Should the physical meaning be  reflected here?
    In the Durak, this card has a physical meaning
    that   it   is   bigger   than     the    king,
    so      I      made      such       a     rank*/

  Joker = 15
};
// clang-format on

struct CardFace {
  static constexpr std::uint32_t kSuitMask = 0b11;
  static constexpr std::uint32_t kRankMask = 0b1111;
  static constexpr std::uint32_t kRankShift = 2;
  static constexpr std::uint32_t kMinRank = 2;
  static constexpr std::uint32_t kSuitCount = kSuitMask + 1;

  [[nodiscard]] static CardFace from_id(engine::card_id_t id) noexcept;

  [[nodiscard]] engine::card_id_t to_id() const noexcept;

  [[nodiscard]] constexpr bool operator==(const CardFace& rhs) const noexcept =
      default;

  Suit suit{};
  Rank rank{Rank::Two};
};

// A deck is fully described by its lowest rank (the top is always the Ace)
// plus whether the two jokers are present; the card count follows from that.
struct DeckKind {
  Rank lowest{Rank::Two};
  bool jokers{false};

  [[nodiscard]] constexpr bool valid() const noexcept {
    return lowest >= Rank::Two && lowest <= Rank::Ace;
  }

  [[nodiscard]] constexpr engine::card_id_t rank_count() const noexcept {
    return static_cast<engine::card_id_t>(static_cast<int>(Rank::Ace) -
                                          static_cast<int>(lowest) + 1);
  }

  [[nodiscard]] constexpr engine::card_id_t size() const noexcept {
    return static_cast<engine::card_id_t>(
        (rank_count() * CardFace::kSuitCount) + (jokers ? 2U : 0U));
  }

  [[nodiscard]] constexpr bool operator==(const DeckKind& rhs) const noexcept =
      default;
};

namespace deck {
inline constexpr DeckKind kPiquet{.lowest = Rank::Nine};                 // 24
inline constexpr DeckKind kSkat{.lowest = Rank::Seven};                  // 32
inline constexpr DeckKind kThirtySix{.lowest = Rank::Six};               // 36
inline constexpr DeckKind kStandard{.lowest = Rank::Two};                // 52
inline constexpr DeckKind kJokers{.lowest = Rank::Two, .jokers = true};  // 54
}  // namespace deck

}  // namespace durak::rules