#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "action_rule.hpp"
#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"
#include "game_rule.hpp"
#include "game_rule_view.hpp"
#include "modulo.hpp"
#include "player.hpp"
#include "player_view.hpp"
#include "table.hpp"

namespace durak::engine {

class DurakEngine {
  /*====================== Usings/Helpers ======================*/
  using modulo_t = modulo::ModuloRing<modulo_base_t>;

  struct GameStatus {
    bool is_game_running{false};
  };

  struct GameResult {
    std::optional<player_id_t> durak{};
  };

 public:
  /*================= Constructors/Destructors =================*/
  DurakEngine(rule_t first_turn, modulo_base_t next_offset, rule_t deck_init,
              rule_t round_start, rule_t round_end, actions_t open_actions,
              actions_t toss_actions, actions_t beat_actions);

  DurakEngine(const DurakEngine& /*unused*/) = delete;
  DurakEngine(DurakEngine&& /*unused*/) = delete;

  ~DurakEngine() = default;

  /*======================= Assignments ========================*/
  DurakEngine& operator=(const DurakEngine& /*unused*/) = delete;
  DurakEngine& operator=(DurakEngine&& /*unused*/) = delete;

  /*====================== Game Lifetime =======================*/
  void start(players_t players);

  void end() noexcept;

  /*========================= Getters ==========================*/
  const GameStatus& get_status() const noexcept;

  modulo_base_t get_attacker_pos() const noexcept;
  modulo_base_t get_defender_pos() const noexcept;
  std::optional<GameResult> get_last_game_result() const noexcept;

 private:
  /*========================= Friends ==========================*/
  friend class PlayerView;
  friend class GameRuleView;
#ifdef UNIT_TESTING
  friend class DurakEngineTestMediator;
#endif

  /*======================== Game Cycle ========================*/
  void game_cycle();
  void play_round();
  void open_phase();
  void toss_phase();
  void beat_phase();
  bool is_game_over() const noexcept;
  void define_loser() noexcept;

  /*========================= Helpers ==========================*/
  Player& get_player(modulo_t pos);
  Player& get_attacker();
  Player& get_defender();
  GameRuleView rule_view() noexcept;
  void apply_action(player_id_t id, const Move& move, const actions_t& actions);
  bool cycle_player(Player& player, const actions_t& actions);
  [[nodiscard]] bool run_cycle(modulo_t start, const actions_t& actions,
                               std::optional<modulo_t> exclude = std::nullopt);
  [[nodiscard]] bool toss_cycle();
  [[nodiscard]] bool beat_cycle();

  /*======================= Data fields ========================*/
  /*======================== Game Rules ========================*/
  const rule_t k_first_turn_rule_{};
  const modulo_base_t k_next_offset_{};
  const rule_t k_deck_init_rule_{};
  const rule_t k_round_start_rule_{};
  const rule_t k_round_end_rule_{};

  const actions_t k_open_actions_{};
  const actions_t k_toss_actions_{};
  const actions_t k_beat_actions_{};

  /*======================= Game Process =======================*/
  GameStatus game_status_{};

  players_t players_{};

  Table table_{};

  modulo_t attacker_pos_{};
  modulo_t defender_pos_{};

  std::vector<Card> stock_{};
  std::vector<Card> bita_{};
  Card trump_{};

  std::optional<GameResult> last_game_result_{};
};

}  // namespace durak::engine