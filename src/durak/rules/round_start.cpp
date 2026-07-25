#include <include/durak/engine/config.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/rules/round_start.hpp>
#include <include/durak/rules/turn_order.hpp>
#include <memory>

namespace durak::rules {

engine::rule_t RoundStartRule::make() {
  return std::make_unique<RoundStartRule>();
}

void RoundStartRule::apply(engine::GameRuleView& view) {
  if (view.players().empty()) {
    return;
  }

  const auto kAfterAttacker = view.advance_seat(view.get_attacker_pos());
  view.set_defender_pos(first_active_from(view, kAfterAttacker));
}

bool RoundStartRule::is_applicable(const engine::GameRuleView& /*view*/) const {
  return true;
}

engine::rule_id_t RoundStartRule::get_id() const noexcept { return kRuleId; }

}  // namespace durak::rules
