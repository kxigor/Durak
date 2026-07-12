#include <include/durak/engine/table.hpp>

namespace durak::engine {

CardOnTable::CardOnTable(Card attack_card) : attack_card_{attack_card} {}

void CardOnTable::beat(Card beat_card) { beat_card_ = beat_card; }

Card CardOnTable::get_attack_card() const { return attack_card_; }

Card CardOnTable::get_beat_card() const { return beat_card_.value(); }

bool CardOnTable::is_beaten() const { return beat_card_.has_value(); }

void Table::clear() { cards_.clear(); }

const Table::cards_on_table_t& Table::get_cards() const { return cards_; }

}  // namespace durak::engine
