#pragma once

#include <optional>
#include <vector>

#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"
#include "player.hpp"

namespace durak::engine {

class GameRuleView {
 public:
  /*================= Constructors/Destructors =================*/
  explicit GameRuleView(DurakEngine& engine) noexcept;

  GameRuleView(const GameRuleView& /*unused*/) = delete;
  GameRuleView(GameRuleView&& /*unused*/) = default;

  ~GameRuleView() = default;

  /*======================= Assignments ========================*/
  GameRuleView& operator=(const GameRuleView& /*unused*/) = delete;
  GameRuleView& operator=(GameRuleView&& /*unused*/) = delete;

  /*========================== Access ==========================*/
  players_t& players() noexcept;
  [[nodiscard]] const players_t& players() const noexcept;

  Table& table() noexcept;
  [[nodiscard]] const Table& table() const noexcept;

  std::vector<Card>& stock() noexcept;
  [[nodiscard]] const std::vector<Card>& stock() const noexcept;

  std::vector<Card>& bita() noexcept;
  [[nodiscard]] const std::vector<Card>& bita() const noexcept;

  Card& trump() noexcept;
  [[nodiscard]] const Card& trump() const noexcept;

  [[nodiscard]] modulo_base_t get_attacker_pos() const noexcept;
  [[nodiscard]] modulo_base_t get_defender_pos() const noexcept;
  void set_attacker_pos(modulo_base_t pos);
  void set_defender_pos(modulo_base_t pos);

  [[nodiscard]] std::optional<player_id_t> get_durak() const noexcept;

 private:
  /*======================= Data fields ========================*/
  DurakEngine& engine_;
};

}  // namespace durak::engine
