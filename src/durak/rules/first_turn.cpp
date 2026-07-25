#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/first_turn.hpp>
#include <include/durak/rules/game_constants.hpp>
#include <memory>
#include <optional>

namespace durak::rules {

engine::rule_t FirstTurnRule::make() {
  return std::make_unique<FirstTurnRule>();
}

void FirstTurnRule::apply(engine::GameRuleView& view) {
  if (view.players().empty()) {
    return;
  }

  deal_initial_hands(view);
  view.set_attacker_pos(find_first_attacker(view));
}

bool FirstTurnRule::is_applicable(const engine::GameRuleView& /*view*/) const {
  return true;
}

engine::rule_id_t FirstTurnRule::get_id() const noexcept { return kRuleId; }

void FirstTurnRule::deal_initial_hands(engine::GameRuleView& view) {
  auto& stock = view.stock();
  auto& players = view.players();

  for (std::size_t dealt = 0; dealt < kHandSize; ++dealt) {
    for (auto& player : players) {
      if (stock.empty()) {
        return;
      }
      player->add_card(stock.back());
      stock.pop_back();
    }
  }
}

engine::modulo_base_t FirstTurnRule::find_first_attacker(
    const engine::GameRuleView& view) {
  const auto kTrumpSuit = CardFace::from_id(view.trump().id).suit;
  const auto& players = view.players();

  std::optional<Rank> lowest;
  engine::modulo_base_t attacker = 0;

  for (std::size_t seat = 0; seat < players.size(); ++seat) {
    for (const auto& card : players.at(seat)->get_cards()) {
      const auto kFace = CardFace::from_id(card.id);
      if (kFace.suit != kTrumpSuit) {
        continue;
      }
      if (!lowest || kFace.rank < *lowest) {
        lowest = kFace.rank;
        attacker = static_cast<engine::modulo_base_t>(seat);
      }
    }
  }

  return attacker;
}

}  // namespace durak::rules
