#include <cstddef>
#include <include/durak/engine/action_rule.hpp>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/move.hpp>
#include <include/durak/engine/player_view.hpp>
#include <vector>

namespace durak::engine {

PlayerView::PlayerView(DurakEngine& engine, player_id_t player_id,
                       const actions_t& actions) noexcept
    : engine_{engine}, player_id_{player_id}, actions_{actions} {}

std::vector<action_id_t> PlayerView::get_actions() const {
  std::vector<action_id_t> result;
  result.reserve(actions_.size());
  for (const auto& entry : actions_) {
    result.push_back(entry.first);
  }
  return result;
}

bool PlayerView::is_available(action_id_t action_id) const {
  auto it = actions_.find(action_id);
  if (it == actions_.end()) {
    return false;
  }
  const GameRuleView kView{engine_};
  return it->second->is_available(player_id_, kView);
}

bool PlayerView::is_legal(const Move& move) const {
  auto it = actions_.find(move.get_action_id());
  if (it == actions_.end()) {
    return false;
  }
  const GameRuleView kView{engine_};
  return it->second->is_legal(player_id_, move, kView);
}

std::size_t PlayerView::get_stock_size() const noexcept {
  return engine_.stock_.size();
}

Card PlayerView::get_trump() const noexcept { return engine_.trump_; }

modulo_base_t PlayerView::get_attacker_pos() const noexcept {
  return engine_.attacker_pos_.get_num();
}

modulo_base_t PlayerView::get_defender_pos() const noexcept {
  return engine_.defender_pos_.get_num();
}

modulo_base_t PlayerView::get_next_offset() const noexcept {
  return engine_.k_next_offset_;
}

std::vector<std::size_t> PlayerView::get_players_card_counts() const {
  std::vector<std::size_t> counts;
  counts.reserve(engine_.players_.size());
  for (const auto& player : engine_.players_) {
    counts.push_back(player->get_card_count());
  }
  return counts;
}

}  // namespace durak::engine
