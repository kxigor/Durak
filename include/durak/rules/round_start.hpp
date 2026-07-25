#pragma once

#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule.hpp>

namespace durak::rules {
class RoundStartRule : public engine::GameRule {
  static constexpr engine::rule_id_t kRuleId = 3;

 public:
  /*================= Constructors/Destructors =================*/
  RoundStartRule() = default;

  RoundStartRule(const RoundStartRule& /*unused*/) = default;
  RoundStartRule(RoundStartRule&& /*unused*/) = default;
  ~RoundStartRule() override = default;

  /*======================= Assignments ========================*/
  RoundStartRule& operator=(const RoundStartRule& /*unused*/) = default;
  RoundStartRule& operator=(RoundStartRule&& /*unused*/) = default;

  /*========================== Fabric ==========================*/
  [[nodiscard]] static engine::rule_t make();

  /*================= Interface Implementation =================*/
  void apply(engine::GameRuleView& view) override;
  [[nodiscard]] bool is_applicable(
      const engine::GameRuleView& view) const override;
  [[nodiscard]] engine::rule_id_t get_id() const noexcept override;
};
}  // namespace durak::rules
