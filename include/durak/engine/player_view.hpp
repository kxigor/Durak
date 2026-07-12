#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class PlayerView {
 public:
  /*= Constructors/Destructors =*/
  PlayerView(DurakEngine& engine, player_id_t player_id);

  /*========= Getters ==========*/
  std::vector<action_id_t> get_possible_actions() const;
  std::size_t get_stock_size() const;
  Card get_trump() const;
  modulo_base_t get_attacking_pos() const;
  modulo_base_t get_defending_pos() const;
  modulo_base_t get_next_offset() const;
  std::vector<std::size_t> get_players_card_counts() const;

 private:
  /*======= Data fields ========*/
  DurakEngine& engine_;
  player_id_t player_id_;
};

}  // namespace durak::engine