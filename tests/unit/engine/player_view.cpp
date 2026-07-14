#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player_view.hpp>

namespace {

using namespace durak::engine;

struct NullRule : GameRule {
  void apply(GameRuleView& /*view*/) override {}
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 0; }
};

std::unique_ptr<GameRule> null_rule() { return std::make_unique<NullRule>(); }

// Action whose availability is fixed at construction.
struct FixedAction : PlayerAction {
  action_id_t id_;
  bool available_;
  FixedAction(action_id_t id, bool available) : id_{id}, available_{available} {}
  void apply(player_id_t /*id*/, GameRuleView& /*view*/) override {}
  bool is_applicable(player_id_t /*id*/,
                     const GameRuleView& /*view*/) const override {
    return available_;
  }
  action_id_t get_id() const noexcept override { return id_; }
};

struct PlayerViewTest : ::testing::Test {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  GameRuleView setup{engine};  // mutate engine state for the view to read
};

// --- Board readouts ---

TEST_F(PlayerViewTest, ReportsStockAndTrump) {
  setup.stock().push_back(Card{1});
  setup.stock().push_back(Card{2});
  setup.trump() = Card{5};

  actions_t none;
  PlayerView view{engine, 0, none};
  EXPECT_EQ(view.get_stock_size(), 2u);
  EXPECT_EQ(view.get_trump().id, 5);
  EXPECT_EQ(view.get_next_offset(), 1);
}

TEST_F(PlayerViewTest, ReportsPositions) {
  setup.players().push_back(Player{1});
  setup.players().push_back(Player{2});
  setup.players().push_back(Player{3});
  setup.set_attacker_pos(0);
  setup.set_defender_pos(1);

  actions_t none;
  PlayerView view{engine, 0, none};
  EXPECT_EQ(view.get_attacker_pos(), 0);
  EXPECT_EQ(view.get_defender_pos(), 1);
}

TEST_F(PlayerViewTest, ReportsPlayerCardCounts) {
  setup.players().push_back(Player{1});
  setup.players().push_back(Player{2});
  setup.players()[0].add_card(Card{1});
  setup.players()[1].add_card(Card{2});
  setup.players()[1].add_card(Card{3});

  actions_t none;
  PlayerView view{engine, 0, none};
  auto counts = view.get_players_card_counts();
  ASSERT_EQ(counts.size(), 2u);
  EXPECT_EQ(counts[0], 1u);
  EXPECT_EQ(counts[1], 2u);
}

// --- Available actions ---

TEST_F(PlayerViewTest, PossibleActionsFilterByApplicability) {
  actions_t actions;
  actions.emplace(1, std::make_unique<FixedAction>(1, true));
  actions.emplace(2, std::make_unique<FixedAction>(2, false));

  PlayerView view{engine, 0, actions};
  auto available = view.get_possible_actions();

  ASSERT_EQ(available.size(), 1u);
  EXPECT_EQ(available[0], 1u);
}

TEST_F(PlayerViewTest, PossibleActionsEmptyWhenNoneApplicable) {
  actions_t actions;
  actions.emplace(1, std::make_unique<FixedAction>(1, false));

  PlayerView view{engine, 0, actions};
  EXPECT_TRUE(view.get_possible_actions().empty());
}

}  // namespace