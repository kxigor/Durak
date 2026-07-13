#include <include/durak/engine/table.hpp>

namespace durak::engine {

CardOnTable::CardOnTable(Card attack_card) : attack_card_{attack_card} {}

void CardOnTable::beat(Card beat_card) { beat_card_ = beat_card; }

Card CardOnTable::get_attack_card() const { return attack_card_; }

Card CardOnTable::get_beat_card() const { return beat_card_.value(); }

bool CardOnTable::is_beaten() const { return beat_card_.has_value(); }

void Table::clear() { cards_.clear(); }

void Table::add_attack(Card attack_card) {
  cards_.emplace_back(attack_card);
}

void Table::beat(std::size_t index, Card beat_card) {
  cards_.at(index).beat(beat_card);
}

const Table::cards_on_table_t& Table::get_cards() const { return cards_; }

std::size_t Table::size() const { return cards_.size(); }

}  // namespace durak::engine
