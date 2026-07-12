#pragma once

#include <cstdint>
#include <string>

#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class PlayerAction {
 public:
  /*======================= Destructors ========================*/
  virtual ~PlayerAction() = default;

  /*========================= Game API =========================*/
  virtual void apply(player_id_t id, DurakEngine& engine) = 0;
  virtual bool is_applicable(player_id_t id, DurakEngine& engine) const = 0;
  virtual action_id_t get_id() const noexcept = 0;
};

}  // namespace durak::engine