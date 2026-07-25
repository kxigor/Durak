#pragma once

#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule.hpp>

namespace durak::rules {
class RoundEndRule : public engine::GameRule {
  static constexpr engine::rule_id_t kRuleId = 4;

 public:
  /*================= Constructors/Destructors =================*/
  RoundEndRule() = default;

  RoundEndRule(const RoundEndRule& /*unused*/) = default;
  RoundEndRule(RoundEndRule&& /*unused*/) = default;
  ~RoundEndRule() override = default;

  /*======================= Assignments ========================*/
  RoundEndRule& operator=(const RoundEndRule& /*unused*/) = default;
  RoundEndRule& operator=(RoundEndRule&& /*unused*/) = default;

  /*========================== Fabric ==========================*/
  [[nodiscard]] static engine::rule_t make();

  /*================= Interface Implementation =================*/
  void apply(engine::GameRuleView& view) override;
  [[nodiscard]] bool is_applicable(
      const engine::GameRuleView& view) const override;
  [[nodiscard]] engine::rule_id_t get_id() const noexcept override;

 private:
  /*========================= Helpers ==========================*/
  [[nodiscard]] static bool resolve_table(engine::GameRuleView& view);
  static void refill_hands(engine::GameRuleView& view);
  static void advance_roles(engine::GameRuleView& view, bool defended);
};
}  // namespace durak::rules
