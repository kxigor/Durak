#pragma once

#include <memory>

#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class GameRule {
 public:
  /*================= Constructors/Destructors =================*/
  GameRule() = default;
  GameRule(const GameRule& /*unused*/) = delete;
  GameRule(GameRule&& /*unused*/) = delete;
  virtual ~GameRule() = default;

  /*======================= Assignments ========================*/
  GameRule& operator=(const GameRule& /*unused*/) = delete;
  GameRule& operator=(GameRule&& /*unused*/) = delete;

  /*========================= Game API =========================*/
  virtual void apply(GameRuleView& view) = 0;
  [[nodiscard]] virtual bool is_applicable(const GameRuleView& view) const = 0;
  [[nodiscard]] virtual rule_id_t get_id() const noexcept = 0;
};

using rule_t = std::unique_ptr<GameRule>;

}  // namespace durak::engine