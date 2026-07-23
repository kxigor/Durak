#pragma once

#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <random>

#include "card_face.hpp"

namespace durak::rules {
class FirstTurnRule : public engine::GameRule {
  static constexpr engine::rule_id_t kRuleId = 2;

 public:
  /*================= Constructors/Destructors =================*/
  explicit FirstTurnRule(
      DeckKind kind = deck::kThirtySix,
      std::mt19937::result_type seed = std::random_device{}());

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
  /*======================= Data fields ========================*/
  const DeckKind kind_{};
  std::mt19937 gen_;
};
}  // namespace durak::rules