#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>

#include "test_mediator.hpp"

namespace {

using namespace durak::engine;

struct NullRule : GameRule {
  void apply(GameRuleView& /*view*/) override {}
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 0; }
};

// Gives every player a single card at deck init.
struct DealOneRule : GameRule {
  void apply(GameRuleView& view) override {
    for (auto& player : view.players()) player.add_card(Card{1});
  }
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 1; }
};

// Empties the first player's hand — drives the game toward termination.
struct ClearFirstRule : GameRule {
  void apply(GameRuleView& view) override {
    auto& first = view.players().front();
    while (first.has_cards()) first.remove_card(0);
  }
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 2; }
};

std::unique_ptr<GameRule> null_rule() { return std::make_unique<NullRule>(); }

std::vector<Player> make_players(std::initializer_list<player_id_t> ids) {
  std::vector<Player> players;
  for (auto id : ids) players.emplace_back(id);
  return players;
}

// --- Lifetime ---

TEST(EngineTest, StartMarksGameRunning) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  engine.start(make_players({1, 2}));
  EXPECT_TRUE(engine.get_status().is_game_running_);
}

TEST(EngineTest, EndStopsGame) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  engine.start(make_players({1, 2}));
  engine.end();
  EXPECT_FALSE(engine.get_status().is_game_running_);
}

// --- Full game to termination ---

TEST(EngineTest, GameEndsWithLastHolderAsDurak) {
  DurakEngine engine{null_rule(),
                     1,
                     std::make_unique<DealOneRule>(),   // deck init: deal cards
                     null_rule(),                        // round start
                     std::make_unique<ClearFirstRule>(),  // round end: empty p0
                     {},
                     {},
                     {}};
  engine.start(make_players({10, 20}));

  auto result = engine.get_last_game_result();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->durak, player_id_t{20});
}

// --- Offset validation ---

TEST(EngineTest, StartRejectsNonCoprimeOffset) {
  DurakEngine engine{null_rule(), 2,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  EXPECT_THROW(engine.start(make_players({1, 2})), std::invalid_argument);
}

// --- is_game_over ---

TEST(EngineTest, GameOverWhenAtMostOneHolder) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  med.players().push_back(Player{1});
  med.players().push_back(Player{2});
  med.players()[0].add_card(Card{1});
  EXPECT_TRUE(med.is_game_over());  // only one holder

  med.players()[1].add_card(Card{2});
  EXPECT_FALSE(med.is_game_over());  // two holders
}

// --- define_loser ---

TEST(EngineTest, DefineLoserPicksTheHolder) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  med.players().push_back(Player{1});
  med.players().push_back(Player{2});
  med.players()[1].add_card(Card{1});

  med.define_loser();
  ASSERT_TRUE(med.has_result());
  EXPECT_EQ(med.result_durak(), player_id_t{2});
}

TEST(EngineTest, DefineLoserNoDurakWhenAllEmpty) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  med.players().push_back(Player{1});
  med.players().push_back(Player{2});

  med.define_loser();
  ASSERT_TRUE(med.has_result());
  EXPECT_FALSE(med.result_durak().has_value());  // a draw, no durak
}

// --- Positions ---

TEST(EngineTest, ReportsSeatedPositions) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};
  med.players().push_back(Player{1});
  med.players().push_back(Player{2});
  med.players().push_back(Player{3});
  med.seat_positions(0, 1);

  EXPECT_EQ(engine.get_attacker_pos(), 0);
  EXPECT_EQ(engine.get_defender_pos(), 1);
}

}  // namespace