#include <algorithm>
#include <format>
#include <stdexcept>
#include <include/durak/engine/engine.hpp>

namespace durak::engine {

DurakEngine::DurakEngine(rule_t first_turn, modulo_base_t next_offset,
                         rule_t deck_init, rule_t round_start, rule_t round_end,
                         actions_t open_actions, actions_t toss_actions,
                         actions_t beat_actions)
    : k_first_turn_rule_{std::move(first_turn)},
      k_next_offset_{std::move(next_offset)},
      k_deck_init_rule_{std::move(deck_init)},
      k_round_start_rule_{std::move(round_start)},
      k_round_end_rule_{std::move(round_end)},
      k_open_actions_{std::move(open_actions)},
      k_toss_actions_{std::move(toss_actions)},
      k_beat_actions_{std::move(beat_actions)} {}

void DurakEngine::start(std::vector<Player> players) {
  game_status_.is_game_running_ = true;
  players_ = std::move(players);
  table_.clear();

  // Seat positions on a ring sized to the player count. The offset must be
  // coprime with that count, otherwise a walk would skip players.
  auto count = static_cast<modulo_base_t>(players_.size());
  attacker_pos_ = modulo_t{count};
  defender_pos_ = modulo_t{count};
  if (count > 0 && !attacker_pos_.covers_full_ring(k_next_offset_)) {
    throw std::invalid_argument(
        "next_offset must be coprime with the player count");
  }

  auto view = rule_view();
  k_first_turn_rule_->apply(view);
  k_deck_init_rule_->apply(view);

  game_cycle();
}

void DurakEngine::end() { game_status_.is_game_running_ = false; }

const DurakEngine::GameStatus& DurakEngine::get_status() const {
  return game_status_;
}

modulo_base_t DurakEngine::get_attacker_pos() const {
  return attacker_pos_.get_num();
}

modulo_base_t DurakEngine::get_defender_pos() const {
  return defender_pos_.get_num();
}

std::optional<DurakEngine::GameResult> DurakEngine::get_last_game_result() const {
  return last_game_result_;
}

void DurakEngine::game_cycle() {
  while (!is_game_over()) {
    play_round();
  }
  define_loser();
}

void DurakEngine::play_round() {
  auto view = rule_view();
  // start rule must initialize defender
  k_round_start_rule_->apply(view);

  open_phase();
  toss_phase();
  beat_phase();

  k_round_end_rule_->apply(view);
}

void DurakEngine::open_phase() {
  cycle_player(get_attacker(), k_open_actions_);
}

void DurakEngine::toss_phase() {
  while (toss_cycle()) {}
}

void DurakEngine::beat_phase() {
  while (beat_cycle()) {}
}

bool DurakEngine::is_game_over() const {
  return std::ranges::count_if(players_, &Player::has_cards) <= 1;
}

void DurakEngine::define_loser() {
  auto it = std::ranges::find_if(players_, &Player::has_cards);
  last_game_result_ =
      it != players_.end() ? GameResult{it->get_id()} : GameResult{};
}

Player& DurakEngine::get_player(modulo_t pos) {
  return players_.at(pos.as<std::size_t>());
}

Player& DurakEngine::get_attacker() { return get_player(attacker_pos_); }

Player& DurakEngine::get_defender() { return get_player(defender_pos_); }

GameRuleView DurakEngine::rule_view() { return GameRuleView{*this}; }

void DurakEngine::apply_action(player_id_t id, action_id_t action_id,
                               const actions_t& actions) {
  if (actions.contains(action_id)) {
    auto view = rule_view();
    actions.at(action_id)->apply(id, view);
  }
}

bool DurakEngine::cycle_player(Player& player, const actions_t& actions) {
  bool acted = false;
  PlayerView view{*this, player.get_id(), actions};
  while (const auto action_id = player.get_action(view)) {
    apply_action(player.get_id(), *action_id, actions);
    acted = true;
  }
  return acted;
}

bool DurakEngine::run_cycle(modulo_t start, const actions_t& actions,
                            std::optional<modulo_t> exclude) {
  bool acted = false;
  auto pos = start;
  for (std::size_t i = 0; i < players_.size(); ++i, pos += k_next_offset_) {
    if (exclude && pos == *exclude) continue;
    acted |= cycle_player(get_player(pos), actions);
  }
  return acted;
}

bool DurakEngine::toss_cycle() {
  return run_cycle(attacker_pos_, k_toss_actions_, defender_pos_);
}

bool DurakEngine::beat_cycle() {
  return run_cycle(defender_pos_, k_beat_actions_);
}

}  // namespace durak::engine
