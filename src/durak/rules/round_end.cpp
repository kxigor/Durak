#include <algorithm>
#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/table.hpp>
#include <include/durak/rules/game_constants.hpp>
#include <include/durak/rules/round_end.hpp>
#include <include/durak/rules/turn_order.hpp>
#include <memory>
#include <vector>

namespace durak::rules {

engine::rule_t RoundEndRule::make() { return std::make_unique<RoundEndRule>(); }

void RoundEndRule::apply(engine::GameRuleView& view) {
  if (view.players().empty()) {
    return;
  }

  const auto kDefended = resolve_table(view);
  refill_hands(view);
  advance_roles(view, kDefended);
}

bool RoundEndRule::is_applicable(const engine::GameRuleView& /*view*/) const {
  return true;
}

engine::rule_id_t RoundEndRule::get_id() const noexcept { return kRuleId; }

bool RoundEndRule::resolve_table(engine::GameRuleView& view) {
  auto& table = view.table();
  const auto& cards = table.get_cards();

  const auto kDefended = std::ranges::all_of(
      cards, [](const engine::CardOnTable& card) { return card.is_beaten(); });

  if (kDefended) {
    auto& bita = view.bita();
    for (const auto& card : cards) {
      bita.push_back(card.get_attack_card());
      bita.push_back(card.get_beat_card());
    }
  } else {
    auto& defender =
        *view.players().at(static_cast<std::size_t>(view.get_defender_pos()));
    for (const auto& card : cards) {
      defender.add_card(card.get_attack_card());
      if (card.is_beaten()) {
        defender.add_card(card.get_beat_card());
      }
    }
  }

  table.clear();
  return kDefended;
}

void RoundEndRule::refill_hands(engine::GameRuleView& view) {
  auto& stock = view.stock();
  auto& players = view.players();
  const auto kDefender = view.get_defender_pos();

  auto refill_seat = [&](engine::modulo_base_t seat) {
    auto& player = *players.at(static_cast<std::size_t>(seat));
    while (player.get_card_count() < kHandSize && !stock.empty()) {
      player.add_card(stock.back());
      stock.pop_back();
    }
  };

  auto pos = view.get_attacker_pos();
  for (std::size_t step = 0; step < players.size(); ++step) {
    if (pos != kDefender) {
      refill_seat(pos);
    }
    pos = view.advance_seat(pos);
  }
  refill_seat(kDefender);
}

void RoundEndRule::advance_roles(engine::GameRuleView& view, bool defended) {
  const auto kDefender = view.get_defender_pos();
  const auto kStart = defended ? kDefender : view.advance_seat(kDefender);

  view.set_attacker_pos(first_active_from(view, kStart));
}

}  // namespace durak::rules
