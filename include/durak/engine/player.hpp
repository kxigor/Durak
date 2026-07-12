#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "card.hpp"
#include "engine_fwd.hpp"
#include "player_view.hpp"

namespace durak::engine {

class Player {
 public:
  /*================= Constructors/Destructors =================*/
  explicit Player(player_id_t id);

  Player(const Player& /*unused*/) = delete;
  Player(Player&& /*unused*/) = default;

  ~Player() = default;

  /*======================= Assignments ========================*/
  Player& operator=(const Player& /*unused*/) = delete;
  Player& operator=(Player&& /*unused*/) = default;

  /*========================= Getters ==========================*/
  player_id_t get_id() const;
  std::size_t get_card_count() const;
  bool has_cards() const;

  /*======================== Modifiers =========================*/
  void add_card(Card card);

  /*=========================== Game ===========================*/
  std::optional<action_id_t> get_action(const PlayerView& view) const;

 private:
  /*======================= Data fields ========================*/
  const player_id_t id_{};
  std::vector<Card> cards_{};
};
}  // namespace durak::engine