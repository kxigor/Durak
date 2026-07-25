#include <cstddef>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/rules/turn_order.hpp>

namespace durak::rules {

engine::modulo_base_t first_active_from(const engine::GameRuleView& view,
                                        engine::modulo_base_t start) {
  const auto& players = view.players();
  if (players.empty()) {
    return 0;
  }

  auto pos = start;
  for (std::size_t step = 0; step < players.size(); ++step) {
    if (players.at(static_cast<std::size_t>(pos))->has_cards()) {
      return pos;
    }
    pos = view.advance_seat(pos);
  }

  return pos;
}

}  // namespace durak::rules
