#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/player_view.hpp>

namespace durak::engine {

PlayerView::PlayerView(DurakEngine& engine, player_id_t player_id)
    : engine_{engine}, player_id_{player_id} {}

std::vector<action_id_t> PlayerView::get_possible_actions() const {
  return {};
}

std::size_t PlayerView::get_stock_size() const {
  return engine_.stock_.size();
}

Card PlayerView::get_trump() const { return engine_.trump_; }

modulo_base_t PlayerView::get_attacking_pos() const {
  return engine_.attacker_pos_.get_num();
}

modulo_base_t PlayerView::get_defending_pos() const {
  return engine_.defender_pos_.get_num();
}

modulo_base_t PlayerView::get_next_offset() const {
  return engine_.k_next_offset_;
}

std::vector<std::size_t> PlayerView::get_players_card_counts() const {
  return {};
}

}  // namespace durak::engine
