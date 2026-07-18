#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/player.hpp>
#include <vector>

namespace durak::engine {

Player::Player(player_id_t id) noexcept : id_{id} {}

player_id_t Player::get_id() const noexcept { return id_; }

std::size_t Player::get_card_count() const noexcept { return cards_.size(); }

bool Player::has_cards() const noexcept { return get_card_count() > 0; }

const std::vector<Card>& Player::get_cards() const noexcept { return cards_; }

void Player::add_card(Card card) { cards_.push_back(card); }

Card Player::remove_card(std::size_t index) {
  Card card = cards_.at(index);
  cards_.erase(cards_.begin() + static_cast<std::ptrdiff_t>(index));
  return card;
}

}  // namespace durak::engine
