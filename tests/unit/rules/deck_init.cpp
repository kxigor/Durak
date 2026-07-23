#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/rules/deck_init.hpp>
#include <memory>
#include <numeric>
#include <vector>

namespace {

using namespace durak::rules;
using durak::engine::Card;
using durak::engine::card_id_t;

struct NullRule : durak::engine::GameRule {
  void apply(durak::engine::GameRuleView& /*view*/) override {}
  bool is_applicable(
      const durak::engine::GameRuleView& /*view*/) const override {
    return true;
  }
  durak::engine::rule_id_t get_id() const noexcept override { return 0; }
};

std::unique_ptr<durak::engine::GameRule> null_rule() {
  return std::make_unique<NullRule>();
}

std::vector<card_id_t> ids_of(const std::vector<Card>& cards) {
  std::vector<card_id_t> ids;
  ids.reserve(cards.size());
  for (const auto& card : cards) ids.push_back(card.id);
  return ids;
}

struct DeckInitTest : ::testing::Test {
  durak::engine::DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                                    null_rule(), {}, {},          {}};
  durak::engine::GameRuleView view{engine};
};

// --- CardFace codec ---

TEST(CardFaceTest, RoundTripsEverySuitAndRank) {
  for (auto suit : {Suit::Clubs, Suit::Diamonds, Suit::Hearts, Suit::Spades}) {
    for (int rank = 2; rank <= 15; ++rank) {
      CardFace face{.suit = suit, .rank = static_cast<Rank>(rank)};
      EXPECT_EQ(CardFace::from_id(face.to_id()), face);
    }
  }
}

TEST(CardFaceTest, EncodesSuitInLowBitsAndRankAbove) {
  EXPECT_EQ((CardFace{.suit = Suit::Clubs, .rank = Rank::Two}).to_id(),
            card_id_t{0});
  EXPECT_EQ((CardFace{.suit = Suit::Spades, .rank = Rank::Two}).to_id(),
            card_id_t{3});
  EXPECT_EQ((CardFace{.suit = Suit::Clubs, .rank = Rank::Three}).to_id(),
            card_id_t{4});
  EXPECT_EQ((CardFace{.suit = Suit::Spades, .rank = Rank::Joker}).to_id(),
            card_id_t{55});
}

TEST(CardFaceTest, DistinctFacesGetDistinctIds) {
  EXPECT_NE((CardFace{.suit = Suit::Clubs, .rank = Rank::Three}).to_id(),
            (CardFace{.suit = Suit::Diamonds, .rank = Rank::Two}).to_id());
}

// --- DeckInitRule identity ---

TEST(DeckInitRuleTest, ReportsItsId) {
  DeckInitRule rule{};
  EXPECT_EQ(rule.get_id(), 1u);
}

TEST(DeckInitRuleTest, MakeBuildsRule) {
  auto rule = DeckInitRule::make(deck::kStandard);
  ASSERT_TRUE(rule);
  EXPECT_EQ(rule->get_id(), 1u);
}

TEST(DeckInitRuleTest, RejectsInvalidDeckKind) {
  EXPECT_THROW(DeckInitRule(DeckKind{static_cast<Rank>(1)}),
               std::invalid_argument);
}

// --- DeckInitRule::apply ---

TEST_F(DeckInitTest, ApplicableOnAnyView) {
  DeckInitRule rule{};
  EXPECT_TRUE(rule.is_applicable(view));
}

TEST_F(DeckInitTest, ApplyDealsEveryIdOnceForEachDeckKind) {
  for (auto kind : {deck::kPiquet, deck::kSkat, deck::kThirtySix,
                    deck::kStandard, deck::kJokers}) {
    DeckInitRule rule{kind, 42};
    rule.apply(view);

    auto ids = ids_of(view.stock());
    std::ranges::sort(ids);

    // Cards run from the deck's lowest rank up to the Ace (ids [lo, 52)),
    // followed by the two jokers when present.
    const auto kLo = static_cast<card_id_t>(
        (static_cast<std::uint32_t>(kind.lowest) - CardFace::kMinRank)
        << CardFace::kRankShift);
    std::vector<card_id_t> expected;
    for (card_id_t id = kLo; id < 52; ++id) expected.push_back(id);
    if (kind.jokers) {
      expected.push_back(52);
      expected.push_back(53);
    }

    EXPECT_EQ(ids.size(), kind.size());
    EXPECT_EQ(ids, expected);
  }
}

TEST_F(DeckInitTest, ApplyShufflesStock) {
  DeckInitRule rule{deck::kThirtySix, 42};
  rule.apply(view);
  auto shuffled = ids_of(view.stock());

  auto sorted = shuffled;
  std::ranges::sort(sorted);
  EXPECT_NE(shuffled, sorted);  // astronomically unlikely to stay ordered
}

TEST_F(DeckInitTest, ApplySameSeedReproducesOrder) {
  DeckInitRule first{deck::kThirtySix, 7};
  first.apply(view);
  auto first_ids = ids_of(view.stock());

  DeckInitRule second{deck::kThirtySix, 7};
  second.apply(view);
  EXPECT_EQ(ids_of(view.stock()), first_ids);
}

TEST_F(DeckInitTest, ApplyReplacesPreviousStock) {
  view.stock().push_back(Card{999});
  DeckInitRule rule{deck::kThirtySix, 42};
  rule.apply(view);

  ASSERT_EQ(view.stock().size(), 36u);
  EXPECT_TRUE(std::ranges::none_of(
      view.stock(), [](const Card& card) { return card.id == 999; }));
}

}  // namespace
