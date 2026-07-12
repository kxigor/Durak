#include <include/durak/engine/player.hpp>

namespace durak::engine {

std::optional<action_id_t> Player::get_action(const PlayerView& /*view*/) const {
  return {};
}

}  // namespace durak::engine
