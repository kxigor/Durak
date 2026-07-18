#pragma once

#include <memory>

#include "config.hpp"

namespace durak::engine {

class Move {
 public:
  /*================= Constructors/Destructors =================*/
  explicit Move(action_id_t action_id) noexcept : action_id_{action_id} {}

  Move(const Move& /*unused*/) = delete;
  Move(Move&& /*unused*/) = delete;

  virtual ~Move() = default;

  /*======================= Assignments ========================*/
  Move& operator=(const Move& /*unused*/) = delete;
  Move& operator=(Move&& /*unused*/) = delete;

  /*========================= Getters ==========================*/
  [[nodiscard]] action_id_t get_action_id() const noexcept {
    return action_id_;
  }

 private:
  /*======================= Data fields ========================*/
  const action_id_t action_id_{};
};

using move_t = std::unique_ptr<Move>;

}  // namespace durak::engine
