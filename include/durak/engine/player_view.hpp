#pragma once

#include <cstddef>
#include <vector>

#include "action_rule.hpp"
#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class PlayerView {
 public:
  /*================= Constructors/Destructors =================*/
  PlayerView(DurakEngine& engine, player_id_t player_id,
             const actions_t& actions) noexcept;

  /*========================= Getters ==========================*/
  [[nodiscard]] std::vector<action_id_t> get_actions() const;
  [[nodiscard]] bool is_available(action_id_t action_id) const;
  [[nodiscard]] bool is_legal(const Move& move) const;

  [[nodiscard]] std::size_t get_stock_size() const noexcept;
  [[nodiscard]] Card get_trump() const noexcept;
  [[nodiscard]] modulo_base_t get_attacker_pos() const noexcept;
  [[nodiscard]] modulo_base_t get_defender_pos() const noexcept;
  [[nodiscard]] modulo_base_t get_next_offset() const noexcept;
  [[nodiscard]] std::vector<std::size_t> get_players_card_counts() const;

 private:
  /*======================= Data fields ========================*/
  DurakEngine& engine_;
  player_id_t player_id_;
  const actions_t& actions_;
};

}  // namespace durak::engine
