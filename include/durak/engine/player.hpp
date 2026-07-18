#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"
#include "move.hpp"
#include "player_view.hpp"

namespace durak::engine {

class Player {
 public:
  /*================= Constructors/Destructors =================*/
  explicit Player(player_id_t id) noexcept;

  Player(const Player& /*unused*/) = delete;
  Player(Player&& /*unused*/) = delete;

  virtual ~Player() = default;

  /*======================= Assignments ========================*/
  Player& operator=(const Player& /*unused*/) = delete;
  Player& operator=(Player&& /*unused*/) = delete;

  /*========================= Getters ==========================*/
  [[nodiscard]] player_id_t get_id() const noexcept;
  [[nodiscard]] std::size_t get_card_count() const noexcept;
  [[nodiscard]] bool has_cards() const noexcept;
  [[nodiscard]] const std::vector<Card>& get_cards() const noexcept;

  /*======================== Modifiers =========================*/
  void add_card(Card card);
  Card remove_card(std::size_t index);

  /*=========================== Game ===========================*/
  [[nodiscard]] virtual move_t get_action(const PlayerView& view) const = 0;

 private:
  /*======================= Data fields ========================*/
  const player_id_t id_{};
  std::vector<Card> cards_{};
};

using player_t = std::unique_ptr<Player>;
using players_t = std::vector<player_t>;

}  // namespace durak::engine
