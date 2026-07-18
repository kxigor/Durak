#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/table.hpp>
#include <optional>

namespace durak::engine {

CardOnTable::CardOnTable(Card attack_card) noexcept
    : attack_card_{attack_card} {}

void CardOnTable::beat(Card beat_card) noexcept { beat_card_ = beat_card; }

Card CardOnTable::get_attack_card() const noexcept { return attack_card_; }

Card CardOnTable::get_beat_card() const {
  if (!beat_card_.has_value()) {
    throw std::bad_optional_access{};
  }
  return *beat_card_;
}

bool CardOnTable::is_beaten() const noexcept { return beat_card_.has_value(); }

void Table::clear() noexcept { cards_.clear(); }

void Table::add_attack(Card attack_card) { cards_.emplace_back(attack_card); }

void Table::beat(std::size_t index, Card beat_card) {
  cards_.at(index).beat(beat_card);
}

const Table::cards_on_table_t& Table::get_cards() const noexcept {
  return cards_;
}

std::size_t Table::size() const noexcept { return cards_.size(); }

}  // namespace durak::engine
