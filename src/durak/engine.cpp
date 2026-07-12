#include <format>
#include <include/durak/engine.hpp>

namespace durak::engine {

CardOnTable::CardOnTable(Card attack_card) : attack_card_{attack_card} {}

void CardOnTable::beat(Card beat_card) { beat_card_ = beat_card; }

Card CardOnTable::get_attack_card() const { return attack_card_; }

Card CardOnTable::get_beat_card() const { return beat_card_.value(); }

bool CardOnTable::is_beaten() const { return beat_card_.has_value(); }

void Table::clear() { cards_.clear(); }

const Table::cards_on_table_t& Table::get_cards() const { return cards_; }

DurakEngine::DurakEngine(rule_t first_attacker, modulo_base_t next_offset,
                         rule_t init_stock, rules_t leading_rules,
                         rules_t attacking_rules, rules_t defending_rules)
    : k_first_attacker_rule_{std::move(first_attacker)},
      k_next_offset_{std::move(next_offset)},
      k_init_stock_rule_{std::move(init_stock)},
      k_leading_rules_{std::move(leading_rules)},
      k_attacking_rules_{std::move(attacking_rules)},
      k_defending_rules_{std::move(defending_rules)} {}

auto DurakEngine::get_last_game_result() const -> std::optional<GameResult> {
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
}  // namespace durak::engine