#pragma once

// Test-only access to DurakEngine internals. Befriended by DurakEngine under
// the UNIT_TESTING flag. Support header (not a 1:1 test file).
#ifndef UNIT_TESTING
#error "test_mediator.hpp requires the UNIT_TESTING build"
#endif

#include <optional>
#include <vector>

#include <include/durak/engine/engine.hpp>

namespace durak::engine {

class DurakEngineTestMediator {
 public:
  explicit DurakEngineTestMediator(DurakEngine& engine) : engine_{engine} {}

  /*======================= State access =======================*/
  std::vector<Player>& players() { return engine_.players_; }
  std::vector<Card>& stock() { return engine_.stock_; }
  std::vector<Card>& bita() { return engine_.bita_; }
  Card& trump() { return engine_.trump_; }

  void seat_positions(modulo_base_t attacker, modulo_base_t defender) {
    auto count = static_cast<modulo_base_t>(engine_.players_.size());
    engine_.attacker_pos_ = DurakEngine::modulo_t{count, attacker};
    engine_.defender_pos_ = DurakEngine::modulo_t{count, defender};
  }

  /*======================= Game result =======================*/
  void set_last_durak(std::optional<player_id_t> durak) {
    engine_.last_game_result_ = DurakEngine::GameResult{durak};
  }
  bool has_result() const { return engine_.last_game_result_.has_value(); }
  std::optional<player_id_t> result_durak() const {
    return engine_.last_game_result_ ? engine_.last_game_result_->durak
                                     : std::nullopt;
  }

  /*===================== Private methods ======================*/
  player_id_t player_at(modulo_base_t pos) {
    return engine_.get_player(DurakEngine::modulo_t{count(), pos}).get_id();
  }
  bool is_game_over() const { return engine_.is_game_over(); }
  void define_loser() { engine_.define_loser(); }
  void game_cycle() { engine_.game_cycle(); }
  void play_round() { engine_.play_round(); }

 private:
  modulo_base_t count() const {
    return static_cast<modulo_base_t>(engine_.players_.size());
  }

  DurakEngine& engine_;
};

}  // namespace durak::engine