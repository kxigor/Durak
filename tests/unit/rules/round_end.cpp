#include <gtest/gtest.h>

#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/player_view.hpp>
#include <include/durak/engine/table.hpp>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/round_end.hpp>
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

struct RoundEndTest : ::testing::Test {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  GameRuleView view{engine};

  void seat_two_with_hands() {
    for (player_id_t id = 0; id < 2; ++id) {
      view.players().push_back(std::make_unique<StubPlayer>(id));
    }
    for (auto& player : view.players()) {
      for (int i = 0; i < 3; ++i) {
        player->add_card(card_of(Suit::Spades, Rank::Ten));
      }
    }
    for (int i = 0; i < 10; ++i) {
      view.stock().push_back(card_of(Suit::Hearts, Rank::Nine));
    }
    view.set_attacker_pos(0);
    view.set_defender_pos(1);
  }
};

TEST_F(RoundEndTest, DefendedSweepsTableToBitaAndDefenderAttacks) {
  seat_two_with_hands();

  auto& table = view.table();
  table.add_attack(card_of(Suit::Clubs, Rank::Six));
  table.beat(0, card_of(Suit::Clubs, Rank::King));
  table.add_attack(card_of(Suit::Diamonds, Rank::Seven));
  table.beat(1, card_of(Suit::Diamonds, Rank::Nine));

  RoundEndRule{}.apply(view);

  EXPECT_EQ(view.table().size(), 0u);
  EXPECT_EQ(view.bita().size(), 4u);
  EXPECT_EQ(static_cast<int>(view.get_attacker_pos()), 1);
  for (const auto& player : view.players()) {
    EXPECT_EQ(player->get_card_count(), 6u);
  }
}

TEST_F(RoundEndTest, PickupGivesTableToDefenderAndSkipsThem) {
  seat_two_with_hands();

  auto& table = view.table();
  table.add_attack(card_of(Suit::Clubs, Rank::Six));
  table.beat(0, card_of(Suit::Clubs, Rank::King));
  table.add_attack(card_of(Suit::Diamonds, Rank::Seven));

  RoundEndRule{}.apply(view);

  EXPECT_EQ(view.table().size(), 0u);
  EXPECT_TRUE(view.bita().empty());
  EXPECT_EQ(view.players()[1]->get_card_count(), 6u);
  EXPECT_EQ(static_cast<int>(view.get_attacker_pos()), 0);
}

TEST(RoundEndRuleTest, ReportsIdAndMakes) {
  auto rule = RoundEndRule::make();
  ASSERT_TRUE(rule);
  EXPECT_EQ(rule->get_id(), 4u);
}

}  // namespace
