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

Player& DurakEngine::get_attacker() {
  return players_.at(attacker_pos_.as<std::size_t>());
}

PlayerView DurakEngine::view_for(const Player& player) {
  return PlayerView{*this, player.get_id()};
}

void DurakEngine::apply_action(player_id_t id, action_id_t action_id,
                               const actions_t& actions) {                    
  if (actions.contains(action_id)) {
    actions.at(action_id)->apply(id, *this);
  }
}

void DurakEngine::open_phase() {
  k_deal_rule_->apply(*this);
  auto& player = get_attacker();

  while (const auto action_id = player.get_action(view_for(player))) {
    apply_action(player.get_id(), *action_id, k_open_actions_);
  }

  toss_phase();
}

void DurakEngine::toss_phase() {
  // TODO: impl
  beat_phase();
}

void DurakEngine::beat_phase() {
  // TODO: impl
}

}  // namespace durak::engine