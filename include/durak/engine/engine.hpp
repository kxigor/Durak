#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "config.hpp"
#include "engine_fwd.hpp"
#include "game_rule.hpp"
#include "modulo.hpp"
#include "player.hpp"
#include "player_action.hpp"
#include "player_view.hpp"
#include "table.hpp"

namespace durak::engine {

class DurakEngine {
  /*====================== Usings/Helpers ======================*/
  using modulo_t = modulo::ModuloRing<modulo_base_t>;

  using rule_t = std::unique_ptr<GameRule>;
  using action_t = std::unique_ptr<PlayerAction>;
  using actions_t = std::vector<action_t>;

  struct GameStatus {
    bool is_game_running_{false};
  };

  struct GameResult {
    std::optional<player_id_t> durak{};
  };

 public:
  /*================= Constructors/Destructors =================*/
  DurakEngine(rule_t first_attacker, modulo_base_t next_offset,
              rule_t init_stock, rule_t leading_stock,
              actions_t leading_actions, actions_t attacking_actions,
              actions_t defending_actions);

  DurakEngine(const DurakEngine& /*unused*/) = delete;
  DurakEngine(DurakEngine&& /*unused*/) = delete;

  ~DurakEngine() = default;

  /*======================= Assignments ========================*/
  DurakEngine& operator=(const DurakEngine& /*unused*/) = delete;
  DurakEngine& operator=(DurakEngine&& /*unused*/) = delete;

  /*====================== Game Lifetime =======================*/
  void start(std::vector<Player> players);

  void end();

  /*========================= Getters ==========================*/
  const GameStatus& get_status() const;

  std::optional<GameResult> get_last_game_result() const;

  /*======================== Game Cycle ========================*/
  void leading_phase() {
    k_leading_stock_rule_->apply(*this);
    auto& player = get_attack_player();

    while (true) {
      PlayerView view{*this, player.get_id()};
      auto action_opt = player.get_action(view);
      if (not action_opt.has_value()) {
        break;
      }
      for (const auto& action : k_leading_actions_) {
        if (action->get_id() == action_opt.value()) {
          action->apply(player.get_id(), *this);
          break;
        }
      }
    }

    attacking_phase();
  }

  void attacking_phase() {
    // TODO: impl

    defending_phase();
  }

  void defending_phase() {
    // TODO: impl
  }

 private:
  /*========================= Helpers ==========================*/
  Player& get_attack_player() {
    return players_.at(attacking_pos_.as<std::size_t>());
  }

  /*======================= Data fields ========================*/
  /*======================== Game Rules ========================*/
  const rule_t k_first_attacker_rule_{};
  const modulo_base_t k_next_offset_{};
  const rule_t k_init_stock_rule_{};
  const rule_t k_leading_stock_rule_{};

  const actions_t k_leading_actions_{};
  const actions_t k_attacking_actions_{};
  const actions_t k_defending_actions_{};

  /*======================= Game Process =======================*/
  GameStatus game_status_{};

  std::vector<Player> players_{};

  Table table_{};

  modulo_t attacking_pos_{};
  modulo_t defending_pos_{};

  std::vector<Card> stock_{};
  Card trump_{};

  std::optional<GameResult> last_game_result_{};
};

};  // namespace durak::engine