#include <format>
#include <include/durak/engine/engine.hpp>

namespace durak::engine {

CardOnTable::CardOnTable(Card attack_card) : attack_card_{attack_card} {}

void CardOnTable::beat(Card beat_card) { beat_card_ = beat_card; }

Card CardOnTable::get_attack_card() const { return attack_card_; }

Card CardOnTable::get_beat_card() const { return beat_card_.value(); }

bool CardOnTable::is_beaten() const { return beat_card_.has_value(); }

void Table::clear() { cards_.clear(); }

const Table::cards_on_table_t& Table::get_cards() const { return cards_; }

DurakEngine::DurakEngine(rule_t first_attacker, modulo_base_t next_offset,
                         rule_t init_stock, rule_t leading_stock,
                         actions_t leading_actions, actions_t attacking_actions,
                         actions_t defending_actions)
    : k_first_attacker_rule_{std::move(first_attacker)},
      k_next_offset_{std::move(next_offset)},
      k_init_stock_rule_{std::move(init_stock)},
      k_leading_stock_rule_{std::move(leading_stock)},
      k_leading_actions_{std::move(leading_actions)},
      k_attacking_actions_{std::move(attacking_actions)},
      k_defending_actions_{std::move(defending_actions)} {}

std::optional<DurakEngine::GameResult> DurakEngine::get_last_game_result()
    const {
  return last_game_result_;
}

void DurakEngine::start(std::vector<Player> players) {
  game_status_.is_game_running_ = true;
  players_ = std::move(players);
  table_.clear();

  k_first_attacker_rule_->apply(*this);
  k_init_stock_rule_->apply(*this);
}

void DurakEngine::end() { game_status_.is_game_running_ = false; }

const DurakEngine::GameStatus& DurakEngine::get_status() const {
  return game_status_;
}

DurakEngine::modulo_base_t DurakEngine::get_attacking_player_pos() const {
  return attacking_pos_.get_num();
}

DurakEngine::modulo_base_t DurakEngine::get_defending_player_pos() const {
  return defending_pos_.get_num();
}
DurakEngine::PlayerView::PlayerView(DurakEngine& engine,
                                    Player::player_id_t player_id)
    : engine_{engine}, player_id_{player_id} {}

std::vector<std::string> DurakEngine::PlayerView::get_possible_actions() const {
  return {};
}

std::size_t DurakEngine::PlayerView::get_stock_size() const {
  return engine_.stock_.size();
}

Card DurakEngine::PlayerView::get_trump() const { return engine_.trump_; }

DurakEngine::modulo_base_t DurakEngine::PlayerView::get_attacking_pos() const {
  return engine_.attacking_pos_.get_num();
}

DurakEngine::modulo_base_t DurakEngine::PlayerView::get_defending_pos() const {
  return engine_.defending_pos_.get_num();
}

DurakEngine::modulo_base_t DurakEngine::PlayerView::get_next_offset() const {
  return engine_.k_next_offset_;
}

std::vector<std::size_t> DurakEngine::PlayerView::get_players_card_counts()
    const {
  return {};
}

}  // namespace durak::engine