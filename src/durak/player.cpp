#include <include/durak/engine/player.hpp>

namespace durak::engine {

Player::Player(player_id_t id) : id_{id} {}

player_id_t Player::get_id() const { return id_; }

std::size_t Player::get_card_count() const { return cards_.size(); }

bool Player::has_cards() const { return get_card_count() > 0; }

void Player::add_card(Card card) { cards_.push_back(card); }

std::optional<action_id_t> Player::get_action(const PlayerView& /*view*/) const {
  return {};
}

}  // namespace durak::engine
