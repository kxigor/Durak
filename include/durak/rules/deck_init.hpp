#pragma once

#include <cstdint>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <random>

namespace durak::rules {

// clang-format off
enum class DeckSize : std::uint8_t {
  Piquet      = 24,
  Skat        = 32,
  ThirtySix   = 36,
  Standard    = 52,
  WithJokers  = 54
};

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
  static constexpr engine::card_id_t kSuitMask = 0b11;
  static constexpr engine::card_id_t kRankMask = 0b1111;
  static constexpr engine::card_id_t kRankShift = 2;
  static constexpr engine::card_id_t kMinRank = 2;

  [[nodiscard]] static CardFace from_id(engine::card_id_t id) noexcept;

  [[nodiscard]] engine::card_id_t to_id() const noexcept;

  [[nodiscard]] constexpr bool operator==(const CardFace& rhs) const noexcept =
      default;

  Suit suit{};
  Rank rank{};
};

class DeckInitRule : public engine::GameRule {
  static constexpr engine::rule_id_t kRuleId = 1;

 public:
  /*================= Constructors/Destructors =================*/
  explicit DeckInitRule(
      DeckSize deck_size = DeckSize::ThirtySix,
      std::mt19937::result_type seed = std::random_device{}());

  DeckInitRule(const DeckInitRule& /*unused*/) = default;
  DeckInitRule(DeckInitRule&& /*unused*/) = default;

  /*======================= Assignments ========================*/
  DeckInitRule& operator=(const DeckInitRule& /*unused*/) = default;
  DeckInitRule& operator=(DeckInitRule&& /*unused*/) = default;

  /*========================== Fabric ==========================*/
  [[nodiscard]] static engine::rule_t make(
      DeckSize deck_size = DeckSize::ThirtySix,
      std::mt19937::result_type seed = std::random_device{}());

  /*================= Interface Implementation =================*/
  void apply(engine::GameRuleView& view) override;
  [[nodiscard]] bool is_applicable(
      const engine::GameRuleView& view) const override;
  [[nodiscard]] engine::rule_id_t get_id() const noexcept override;

 private:
  /*======================= Data fields ========================*/
  const DeckSize deck_size_{};
  std::mt19937 gen_{};
};

}  // namespace durak::rules
