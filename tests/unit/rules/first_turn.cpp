#include <gtest/gtest.h>

#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/player_view.hpp>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/first_turn.hpp>
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

Card card_of(Suit suit, Rank rank) {
  return Card{CardFace{.suit = suit, .rank = rank}.to_id()};
}

struct FirstTurnTest : ::testing::Test {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  GameRuleView view{engine};

  void seat(std::size_t count) {
    for (player_id_t id = 0; id < count; ++id) {
      view.players().push_back(std::make_unique<StubPlayer>(id));
    }
  }
};

TEST_F(FirstTurnTest, DealsSixToEachPlayer) {
  seat(3);
  auto& stock = view.stock();
  for (int i = 0; i < 20; ++i) {
    stock.push_back(card_of(Suit::Spades, Rank::Ten));
  }
  view.trump() = card_of(Suit::Clubs, Rank::Ace);

  FirstTurnRule{}.apply(view);

  for (const auto& player : view.players()) {
    EXPECT_EQ(player->get_card_count(), 6u);
  }
  EXPECT_EQ(view.stock().size(), 2u);
}

TEST_F(FirstTurnTest, FirstAttackerHoldsLowestTrump) {
  seat(2);
  view.trump() = card_of(Suit::Clubs, Rank::Ace);

  auto& stock = view.stock();
  for (int i = 0; i < 10; ++i) {
    stock.push_back(card_of(Suit::Spades, Rank::Nine));
  }
  stock.push_back(card_of(Suit::Clubs, Rank::Six));
  stock.push_back(card_of(Suit::Clubs, Rank::King));

  FirstTurnRule{}.apply(view);

  EXPECT_EQ(static_cast<int>(view.get_attacker_pos()), 1);
}

TEST(FirstTurnRuleTest, ReportsIdAndMakes) {
  auto rule = FirstTurnRule::make();
  ASSERT_TRUE(rule);
  EXPECT_EQ(rule->get_id(), 2u);
}

}  // namespace
