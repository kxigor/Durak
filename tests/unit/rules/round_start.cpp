#include <gtest/gtest.h>

#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/player_view.hpp>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/round_start.hpp>
#include <memory>

namespace {

using namespace durak::rules;
using durak::engine::Card;
using durak::engine::DurakEngine;
using durak::engine::GameRule;
using durak::engine::GameRuleView;
using durak::engine::move_t;
using durak::engine::Player;
using durak::engine::player_id_t;
using durak::engine::PlayerView;
using durak::engine::rule_id_t;

struct NullRule : GameRule {
  void apply(GameRuleView& /*view*/) override {}
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 0; }
};

std::unique_ptr<GameRule> null_rule() { return std::make_unique<NullRule>(); }

struct StubPlayer : Player {
  using Player::Player;
  move_t get_action(const PlayerView& /*view*/) const override {
    return nullptr;
  }
};

Card some_card() {
  return Card{CardFace{.suit = Suit::Spades, .rank = Rank::Ten}.to_id()};
}

struct RoundStartTest : ::testing::Test {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  GameRuleView view{engine};

  void seat(std::size_t count) {
    for (player_id_t id = 0; id < count; ++id) {
      view.players().push_back(std::make_unique<StubPlayer>(id));
    }
  }
};

TEST_F(RoundStartTest, DefenderIsNextAfterAttacker) {
  seat(3);
  for (auto& player : view.players()) {
    player->add_card(some_card());
  }
  view.set_attacker_pos(0);

  RoundStartRule{}.apply(view);

  EXPECT_EQ(static_cast<int>(view.get_defender_pos()), 1);
}

TEST_F(RoundStartTest, SkipsPlayersWithoutCards) {
  seat(3);
  view.players()[0]->add_card(some_card());
  view.players()[2]->add_card(some_card());
  view.set_attacker_pos(0);

  RoundStartRule{}.apply(view);

  EXPECT_EQ(static_cast<int>(view.get_defender_pos()), 2);
}

TEST(RoundStartRuleTest, ReportsIdAndMakes) {
  auto rule = RoundStartRule::make();
  ASSERT_TRUE(rule);
  EXPECT_EQ(rule->get_id(), 3u);
}

}  // namespace
