#include <gtest/gtest.h>

#include <algorithm>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/match.hpp>
#include <set>

namespace {
using durak::rules::Match;
using durak::rules::Phase;

TEST(MatchTest, EveryVisibleTurnOffersALegalMoveAndConservesTheDeck) {
  std::set<Phase> phases;
  for (unsigned seed = 0; seed < 80; ++seed) {
    Match match{seed};
    for (int turn = 0; match.phase() != Phase::Finished && turn < 1500;
         ++turn) {
      phases.insert(match.phase());
      const auto moves = match.legal_moves();
      ASSERT_FALSE(moves.empty());
      EXPECT_THROW(match.play(99), std::invalid_argument);
      EXPECT_EQ(moves, match.legal_moves());
      const auto& board = match.board();
      std::set<unsigned> cards;
      auto add = [&](auto card) { EXPECT_TRUE(cards.insert(card.id).second); };
      for (auto card : board.stock()) add(card);
      for (auto card : board.bita()) add(card);
      for (const auto& player : board.players())
        for (auto card : player->get_cards()) add(card);
      for (const auto& pair : board.table().get_cards()) {
        add(pair.get_attack_card());
        if (pair.is_beaten()) add(pair.get_beat_card());
      }
      EXPECT_EQ(cards.size(), 36U);
      match.play(moves.front());
    }
    EXPECT_EQ(match.phase(), Phase::Finished) << seed;
    EXPECT_FALSE(match.result().empty());
  }
  EXPECT_EQ(phases.size(), 4U);
}
}  // namespace
