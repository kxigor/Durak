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

namespace {
TEST(MultiplayerMatch, TwoThroughSixSeatsFinishWithOneLoserOrADraw) {
  EXPECT_THROW((Match{1, 3}), std::invalid_argument);
  EXPECT_THROW((Match{7, 3}), std::invalid_argument);
  for (std::size_t players = 2; players <= 6; ++players) {
    for (unsigned seed = 0; seed < 24; ++seed) {
      Match match{players, seed};
      std::set<std::size_t> actors;
      bool third_player_threw = false;
      for (unsigned turn = 0; turn < 3000 && match.phase() != Phase::Finished;
           ++turn) {
        const auto seat = match.actor();
        ASSERT_LT(seat, players);
        actors.insert(seat);
        ASSERT_FALSE(match.legal_moves().empty());
        if (match.phase() == Phase::Throw &&
            seat != static_cast<std::size_t>(match.board().get_attacker_pos()))
          third_player_threw = true;
        match.play(match.computer_move());
      }
      ASSERT_EQ(match.phase(), Phase::Finished)
          << players << " seats, seed " << seed;
      EXPECT_EQ(actors.size(), players);
      if (players > 2) {
        EXPECT_TRUE(third_player_threw);
      }
      int losers = 0;
      int draws = 0;
      for (std::size_t seat = 0; seat < players; ++seat) {
        const auto result = match.result(seat);
        losers += result == "loss";
        draws += result == "draw";
        EXPECT_EQ(result == "loss",
                  match.board().players().at(seat)->has_cards());
      }
      EXPECT_TRUE((losers == 1 && draws == 0) || (losers == 0 && draws >= 2));
    }
  }
}
}  // namespace
