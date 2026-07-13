#pragma once

#include <memory>
#include <unordered_map>

#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class PlayerAction {
 public:
  /*======================= Destructors ========================*/
  virtual ~PlayerAction() = default;

  /*========================= Game API =========================*/
  virtual void apply(player_id_t id, GameRuleView& view) = 0;
  virtual bool is_applicable(player_id_t id,
                             const GameRuleView& view) const = 0;
  virtual action_id_t get_id() const noexcept = 0;
};

// A phase's action set, keyed by id. Shared by the engine (which drives the
// phases) and PlayerView (which filters it through is_applicable).
using action_t = std::unique_ptr<PlayerAction>;
using actions_t = std::unordered_map<action_id_t, action_t>;

}  // namespace durak::engine
