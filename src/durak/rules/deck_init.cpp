#include <algorithm>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/rules/deck_init.hpp>
#include <memory>
#include <random>

namespace durak::rules {

CardFace CardFace::from_id(engine::card_id_t id) noexcept {
  return CardFace{
      .suit = static_cast<Suit>(id & kSuitMask),
      .rank = static_cast<Rank>(((id >> kRankShift) & kRankMask) + kMinRank)};
}

engine::card_id_t CardFace::to_id() const noexcept {
  return static_cast<engine::card_id_t>(
      static_cast<engine::card_id_t>(suit) |
      ((static_cast<engine::card_id_t>(rank) - kMinRank) << kRankShift));
}

DeckInitRule::DeckInitRule(DeckSize deck_size, std::mt19937::result_type seed)
    : deck_size_{deck_size}, gen_{seed} {}

engine::rule_t DeckInitRule::make(DeckSize deck_size,
                                  std::mt19937::result_type seed) {
  return std::make_unique<DeckInitRule>(deck_size, seed);
}

void DeckInitRule::apply(engine::GameRuleView& view) {
  auto& stock = view.stock();
  stock.clear();

  for (engine::card_id_t i = 0; i < static_cast<engine::card_id_t>(deck_size_);
       ++i) {
    stock.emplace_back(i);
  }

  std::shuffle(stock.begin(), stock.end(), gen_);
}

bool DeckInitRule::is_applicable(const engine::GameRuleView& /*view*/) const {
  return true;
}

engine::rule_id_t DeckInitRule::get_id() const noexcept { return kRuleId; }

}  // namespace durak::rules
