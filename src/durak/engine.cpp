#include <format>
#include <include/durak/engine/engine.hpp>

namespace durak::engine {

DurakEngine::DurakEngine(rule_t first_attacker, modulo_base_t next_offset,
                         rule_t init_stock, rule_t leading_stock,
                         actions_t open_actions, actions_t toss_actions,
                         actions_t beat_actions)
    : k_first_turn_rule_{std::move(first_attacker)},
      k_next_offset_{std::move(next_offset)},
      k_deck_init_rule_{std::move(init_stock)},
      k_deal_rule_{std::move(leading_stock)},
      k_open_actions_{std::move(open_actions)},
      k_toss_actions_{std::move(toss_actions)},
      k_beat_actions_{std::move(beat_actions)} {}

std::optional<DurakEngine::GameResult> DurakEngine::get_last_game_result()
    const {
  return last_game_result_;
}

void DurakEngine::start(std::vector<Player> players) {
  game_status_.is_game_running_ = true;
  players_ = std::move(players);
  table_.clear();

  k_first_turn_rule_->apply(*this);
  k_deck_init_rule_->apply(*this);
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

Player& DurakEngine::get_player(modulo_t pos) {
  return players_.at(pos.as<std::size_t>());
}

Player& DurakEngine::get_attacker() { return get_player(attacker_pos_); }

Player& DurakEngine::get_defender() { return get_player(defender_pos_); }

PlayerView DurakEngine::view_for(const Player& player) {
  return PlayerView{*this, player.get_id()};
}

void DurakEngine::apply_action(player_id_t id, action_id_t action_id,
                               const actions_t& actions) {
  if (actions.contains(action_id)) {
    actions.at(action_id)->apply(id, *this);
  }
}

[[nodiscard]] bool DurakEngine::cycle_player(Player& player,
                                             const actions_t& actions) {
  bool is_action_used = false;

  auto view = view_for(player);
  while (const auto action_id = player.get_action(view)) {
    apply_action(player.get_id(), *action_id, actions);
    is_action_used = true;
  }

  return is_action_used;
}

void DurakEngine::open_phase() {
  k_deal_rule_->apply(*this);

  // open player must do something
  while (not cycle_player(get_attacker(), k_open_actions_));

  toss_phase();
}

void DurakEngine::toss_phase() {

  // process while players toss
  while (toss_cycle());

  beat_phase();
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

[[nodiscard]] bool DurakEngine::toss_cycle() {
  return run_cycle(attacker_pos_, k_toss_actions_, defender_pos_);
}

void DurakEngine::beat_phase() {
  while (beat_cycle());
}

[[nodiscard]] bool DurakEngine::beat_cycle() {
  return run_cycle(defender_pos_, k_beat_actions_);
}

}  // namespace durak::engine