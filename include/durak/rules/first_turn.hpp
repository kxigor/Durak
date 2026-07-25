#pragma once

#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule.hpp>

namespace durak::rules {
class FirstTurnRule : public engine::GameRule {
  static constexpr engine::rule_id_t kRuleId = 2;

 public:
  /*================= Constructors/Destructors =================*/
  FirstTurnRule() = default;

  FirstTurnRule(const FirstTurnRule& /*unused*/) = default;
  FirstTurnRule(FirstTurnRule&& /*unused*/) = default;
  ~FirstTurnRule() override = default;

  /*======================= Assignments ========================*/
  FirstTurnRule& operator=(const FirstTurnRule& /*unused*/) = default;
  FirstTurnRule& operator=(FirstTurnRule&& /*unused*/) = default;

  /*========================== Fabric ==========================*/
  [[nodiscard]] static engine::rule_t make();

  /*================= Interface Implementation =================*/
  void apply(engine::GameRuleView& view) override;
  [[nodiscard]] bool is_applicable(
      const engine::GameRuleView& view) const override;
  [[nodiscard]] engine::rule_id_t get_id() const noexcept override;

 private:
  /*========================= Helpers ==========================*/
  static void deal_initial_hands(engine::GameRuleView& view);
  [[nodiscard]] static engine::modulo_base_t find_first_attacker(
      const engine::GameRuleView& view);
};
}  // namespace durak::rules
