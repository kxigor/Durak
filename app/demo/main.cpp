#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/move.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/player_view.hpp>
#include <memory>
#include <utility>

using durak::engine::DurakEngine;
using durak::engine::GameRule;
using durak::engine::GameRuleView;
using durak::engine::move_t;
using durak::engine::Player;
using durak::engine::player_id_t;
using durak::engine::players_t;
using durak::engine::PlayerView;
using durak::engine::rule_id_t;

namespace {

struct StubRule : GameRule {
  void apply(GameRuleView& /*view*/) override {}
  [[nodiscard]] bool is_applicable(
      const GameRuleView& /*view*/) const override {
    return true;
  }
  [[nodiscard]] rule_id_t get_id() const noexcept override { return 0; }
};

struct StubPlayer : Player {
  using Player::Player;
  [[nodiscard]] move_t get_action(const PlayerView& /*view*/) const override {
    return nullptr;
  }
};

}  // namespace

int main() {
  DurakEngine engine{
      std::make_unique<StubRule>(),
      1,
      std::make_unique<StubRule>(),
      std::make_unique<StubRule>(),
      std::make_unique<StubRule>(),
      {},
      {},
      {},
  };

  players_t players;
  players.push_back(std::make_unique<StubPlayer>(player_id_t{1}));
  players.push_back(std::make_unique<StubPlayer>(player_id_t{2}));

  engine.start(std::move(players));
}
