#include <gtest/gtest.h>

#include <stdexcept>

#include <include/durak/engine/player.hpp>

namespace {

using durak::engine::Card;
using durak::engine::Player;

// --- Identity ---

TEST(PlayerTest, KeepsItsId) {
  Player p{42};
  EXPECT_EQ(p.get_id(), 42u);
}

// --- Empty hand ---

TEST(PlayerTest, StartsWithNoCards) {
  Player p{1};
  EXPECT_EQ(p.get_card_count(), 0u);
  EXPECT_FALSE(p.has_cards());
  EXPECT_TRUE(p.get_cards().empty());
}

// --- Adding cards ---

TEST(PlayerTest, AddCardGrowsHand) {
  Player p{1};
  p.add_card(Card{5});
  p.add_card(Card{6});
  EXPECT_EQ(p.get_card_count(), 2u);
  EXPECT_TRUE(p.has_cards());
  EXPECT_EQ(p.get_cards()[0].id, 5);
  EXPECT_EQ(p.get_cards()[1].id, 6);
}

// --- Removing cards ---

TEST(PlayerTest, RemoveCardReturnsAndShrinks) {
  Player p{1};
  p.add_card(Card{5});
  p.add_card(Card{6});
  Card taken = p.remove_card(0);
  EXPECT_EQ(taken.id, 5);
  EXPECT_EQ(p.get_card_count(), 1u);
  EXPECT_EQ(p.get_cards()[0].id, 6);
}

TEST(PlayerTest, RemoveOutOfRangeThrows) {
  Player p{1};
  EXPECT_THROW((void)p.remove_card(0), std::out_of_range);
}

}  // namespace
