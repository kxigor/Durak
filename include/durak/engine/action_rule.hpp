#pragma once

#include <memory>
#include <unordered_map>

#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class ActionRule {
 public:
  /*================= Constructors/Destructors =================*/
  ActionRule() = default;
  ActionRule(const ActionRule& /*unused*/) = delete;
  ActionRule(ActionRule&& /*unused*/) = delete;
  virtual ~ActionRule() = default;

  /*======================= Assignments ========================*/
  ActionRule& operator=(const ActionRule& /*unused*/) = delete;
  ActionRule& operator=(ActionRule&& /*unused*/) = delete;

  /*========================= Game API =========================*/
  [[nodiscard]] virtual bool is_available(player_id_t id,
                                          const GameRuleView& view) const = 0;
  [[nodiscard]] virtual bool is_legal(player_id_t id, const Move& move,
                                      const GameRuleView& view) const = 0;
  virtual void apply(player_id_t id, const Move& move, GameRuleView& view) = 0;
  [[nodiscard]] virtual action_id_t get_id() const noexcept = 0;
};

using action_t = std::unique_ptr<ActionRule>;
using actions_t = std::unordered_map<action_id_t, action_t>;

}  // namespace durak::engine
