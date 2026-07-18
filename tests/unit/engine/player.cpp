#include <gtest/gtest.h>

#include <include/durak/engine/player.hpp>
#include <stdexcept>

namespace {

using durak::engine::Card;
using durak::engine::move_t;
using durak::engine::Player;
using durak::engine::PlayerView;

struct StubPlayer : Player {
  using Player::Player;
  move_t get_action(const PlayerView& /*view*/) const override {
    return nullptr;
  }
};

// --- Identity ---

TEST(PlayerTest, KeepsItsId) {
  StubPlayer p{42};
  EXPECT_EQ(p.get_id(), 42u);
}

// --- Empty hand ---

TEST(PlayerTest, StartsWithNoCards) {
  StubPlayer p{1};
  EXPECT_EQ(p.get_card_count(), 0u);
  EXPECT_FALSE(p.has_cards());
  EXPECT_TRUE(p.get_cards().empty());
}

// --- Adding cards ---

TEST(PlayerTest, AddCardGrowsHand) {
  StubPlayer p{1};
  p.add_card(Card{5});
  p.add_card(Card{6});
  EXPECT_EQ(p.get_card_count(), 2u);
  EXPECT_TRUE(p.has_cards());
  EXPECT_EQ(p.get_cards()[0].id, 5);
  EXPECT_EQ(p.get_cards()[1].id, 6);
}

// --- Removing cards ---

TEST(PlayerTest, RemoveCardReturnsAndShrinks) {
  StubPlayer p{1};
  p.add_card(Card{5});
  p.add_card(Card{6});
  Card taken = p.remove_card(0);
  EXPECT_EQ(taken.id, 5);
  EXPECT_EQ(p.get_card_count(), 1u);
  EXPECT_EQ(p.get_cards()[0].id, 6);
}

TEST(PlayerTest, RemoveOutOfRangeThrows) {
  StubPlayer p{1};
  EXPECT_THROW((void)p.remove_card(0), std::out_of_range);
}

}  // namespace
