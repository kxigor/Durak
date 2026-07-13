#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player_view.hpp>

namespace durak::engine {

PlayerView::PlayerView(DurakEngine& engine, player_id_t player_id,
                       const actions_t& actions)
    : engine_{engine}, player_id_{player_id}, actions_{actions} {}

std::vector<action_id_t> PlayerView::get_possible_actions() const {
  std::vector<action_id_t> result;
  GameRuleView view{engine_};
  for (const auto& [id, action] : actions_) {
    if (action->is_applicable(player_id_, view)) {
      result.push_back(id);
    }
  }
  return result;
}

std::size_t PlayerView::get_stock_size() const {
  return engine_.stock_.size();
}

Card PlayerView::get_trump() const { return engine_.trump_; }

modulo_base_t PlayerView::get_attacker_pos() const {
  return engine_.attacker_pos_.get_num();
}

modulo_base_t PlayerView::get_defender_pos() const {
  return engine_.defender_pos_.get_num();
}

modulo_base_t PlayerView::get_next_offset() const {
  return engine_.k_next_offset_;
}

std::vector<std::size_t> PlayerView::get_players_card_counts() const {
  std::vector<std::size_t> counts;
  counts.reserve(engine_.players_.size());
  for (const auto& player : engine_.players_) {
    counts.push_back(player.get_card_count());
  }
  return counts;
}

}  // namespace durak::engine
