#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

class GameRuleView {
 public:
  /*================= Constructors/Destructors =================*/
  explicit GameRuleView(DurakEngine& engine);

  GameRuleView(const GameRuleView& /*unused*/) = delete;
  GameRuleView(GameRuleView&& /*unused*/) = default;

  ~GameRuleView() = default;

  /*======================= Assignments ========================*/
  GameRuleView& operator=(const GameRuleView& /*unused*/) = delete;
  GameRuleView& operator=(GameRuleView&& /*unused*/) = delete;

  /*========================== Access ==========================*/
  std::vector<Player>& players();
  const std::vector<Player>& players() const;

  Table& table();
  const Table& table() const;

  std::vector<Card>& stock();
  const std::vector<Card>& stock() const;

  std::vector<Card>& bita();
  const std::vector<Card>& bita() const;

  Card& trump();
  const Card& trump() const;

  modulo_base_t get_attacker_pos() const;
  modulo_base_t get_defender_pos() const;
  void set_attacker_pos(modulo_base_t pos);
  void set_defender_pos(modulo_base_t pos);

  std::optional<player_id_t> get_durak() const;

 private:
  /*======================= Data fields ========================*/
  DurakEngine& engine_;
};

}  // namespace durak::engine
