#include <gtest/gtest.h>

#include <include/durak/engine/table.hpp>
#include <optional>
#include <stdexcept>

namespace {

using durak::engine::Card;
using durak::engine::CardOnTable;
using durak::engine::Table;

// --- CardOnTable: attack only ---

TEST(TableTest, UnbeatenExposesAttack) {
  CardOnTable c{Card{7}};
  EXPECT_EQ(c.get_attack_card().id, 7);
  EXPECT_FALSE(c.is_beaten());
}

TEST(TableTest, GetBeatOnUnbeatenThrows) {
  CardOnTable c{Card{7}};
  EXPECT_THROW((void)c.get_beat_card(), std::bad_optional_access);
}

// --- CardOnTable: beaten ---

TEST(TableTest, BeatStoresCoveringCard) {
  CardOnTable c{Card{7}};
  c.beat(Card{9});
  EXPECT_TRUE(c.is_beaten());
  EXPECT_EQ(c.get_beat_card().id, 9);
  EXPECT_EQ(c.get_attack_card().id, 7);  // attack unchanged
}

// --- Table: empty ---

TEST(TableTest, StartsEmpty) {
  Table t;
  EXPECT_EQ(t.size(), 0u);
  EXPECT_TRUE(t.get_cards().empty());
}

// --- Table: add / read ---

TEST(TableTest, AddAttackGrowsTable) {
  Table t;
  t.add_attack(Card{3});
  t.add_attack(Card{4});
  EXPECT_EQ(t.size(), 2u);
  EXPECT_EQ(t.get_cards()[0].get_attack_card().id, 3);
  EXPECT_EQ(t.get_cards()[1].get_attack_card().id, 4);
}

// --- Table: beat by index ---

TEST(TableTest, BeatCoversCardAtIndex) {
  Table t;
  t.add_attack(Card{3});
  t.beat(0, Card{8});
  EXPECT_TRUE(t.get_cards()[0].is_beaten());
  EXPECT_EQ(t.get_cards()[0].get_beat_card().id, 8);
}

TEST(TableTest, BeatOutOfRangeThrows) {
  Table t;
  t.add_attack(Card{3});
  EXPECT_THROW(t.beat(5, Card{8}), std::out_of_range);
}

// --- Table: clear ---

TEST(TableTest, ClearEmptiesTable) {
  Table t;
  t.add_attack(Card{3});
  t.clear();
  EXPECT_EQ(t.size(), 0u);
}

}  // namespace