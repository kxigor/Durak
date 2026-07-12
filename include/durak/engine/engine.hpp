#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
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
  using actions_t = std::unordered_map<action_id_t, action_t>;

  struct GameStatus {
    bool is_game_running_{false};
  };

  struct GameResult {
    std::optional<player_id_t> durak{};
  };

 public:
  /*================= Constructors/Destructors =================*/
  DurakEngine(rule_t first_attacker, modulo_base_t next_offset,
              rule_t init_stock, rule_t leading_stock, actions_t open_actions,
              actions_t toss_actions, actions_t beat_actions);

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

  modulo_base_t get_attacker_pos() const;
  modulo_base_t get_defender_pos() const;
  std::optional<GameResult> get_last_game_result() const;

  /*======================== Game Cycle ========================*/
  void open_phase();
  void toss_phase();
  void beat_phase();

 private:
  friend class PlayerView;

  /*========================= Helpers ==========================*/
  Player& get_attacker();
  PlayerView view_for(const Player& player);
  void apply_action(player_id_t id, action_id_t action_id,
                    const actions_t& actions);

  /*======================= Data fields ========================*/
  /*======================== Game Rules ========================*/
  const rule_t k_first_turn_rule_{};
  const modulo_base_t k_next_offset_{};
  const rule_t k_deck_init_rule_{};
  const rule_t k_deal_rule_{};

  const actions_t k_open_actions_{};
  const actions_t k_toss_actions_{};
  const actions_t k_beat_actions_{};

  /*======================= Game Process =======================*/
  GameStatus game_status_{};

  std::vector<Player> players_{};

  Table table_{};

  modulo_t attacker_pos_{};
  modulo_t defender_pos_{};

  std::vector<Card> stock_{};
  std::vector<Card> bita_{};
  Card trump_{};

  std::optional<GameResult> last_game_result_{};
};

};  // namespace durak::engine