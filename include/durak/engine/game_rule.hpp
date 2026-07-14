#pragma once

#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class GameRule {
 public:
  /*======================= Destructors ========================*/
  virtual ~GameRule() = default;

  /*========================= Game API =========================*/
  virtual void apply(GameRuleView& view) = 0;
  virtual bool is_applicable(const GameRuleView& view) const = 0;
  virtual rule_id_t get_id() const noexcept = 0;
};

}  // namespace durak::engine