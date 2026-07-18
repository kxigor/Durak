#include <gtest/gtest.h>

#include <algorithm>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player_view.hpp>
#include <memory>
#include <string>

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

struct StubPlayer : Player {
  using Player::Player;
  move_t get_action(const PlayerView& /*view*/) const override {
    return nullptr;
  }
};

struct FixedAction : ActionRule {
  action_id_t id_;
  bool available_;
  bool legal_;
  FixedAction(action_id_t id, bool available, bool legal)
      : id_{id}, available_{available}, legal_{legal} {}
  bool is_available(player_id_t /*id*/,
                    const GameRuleView& /*view*/) const override {
    return available_;
  }
  bool is_legal(player_id_t /*id*/, const Move& /*move*/,
                const GameRuleView& /*view*/) const override {
    return legal_;
  }
  void apply(player_id_t /*id*/, const Move& /*move*/,
             GameRuleView& /*view*/) override {}
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
  setup.players().push_back(std::make_unique<StubPlayer>(1));
  setup.players().push_back(std::make_unique<StubPlayer>(2));
  setup.players().push_back(std::make_unique<StubPlayer>(3));
  setup.set_attacker_pos(0);
  setup.set_defender_pos(1);

  actions_t none;
  PlayerView view{engine, 0, none};
  EXPECT_EQ(view.get_attacker_pos(), 0);
  EXPECT_EQ(view.get_defender_pos(), 1);
}

TEST_F(PlayerViewTest, ReportsPlayerCardCounts) {
  setup.players().push_back(std::make_unique<StubPlayer>(1));
  setup.players().push_back(std::make_unique<StubPlayer>(2));
  setup.players()[0]->add_card(Card{1});
  setup.players()[1]->add_card(Card{2});
  setup.players()[1]->add_card(Card{3});

  actions_t none;
  PlayerView view{engine, 0, none};
  auto counts = view.get_players_card_counts();
  ASSERT_EQ(counts.size(), 2u);
  EXPECT_EQ(counts[0], 1u);
  EXPECT_EQ(counts[1], 2u);
}

// --- Actions and probes ---

TEST_F(PlayerViewTest, GetActionsReturnsEveryVerbUnfiltered) {
  actions_t actions;
  actions.emplace(1, std::make_unique<FixedAction>(1, true, true));
  actions.emplace(2, std::make_unique<FixedAction>(2, false, false));

  PlayerView view{engine, 0, actions};
  auto all = view.get_actions();

  ASSERT_EQ(all.size(), 2u);  // unavailable verbs are still listed
  EXPECT_NE(std::ranges::find(all, 1u), all.end());
  EXPECT_NE(std::ranges::find(all, 2u), all.end());
}

TEST_F(PlayerViewTest, IsAvailableReflectsRuleAndUnknownIsFalse) {
  actions_t actions;
  actions.emplace(1, std::make_unique<FixedAction>(1, true, true));
  actions.emplace(2, std::make_unique<FixedAction>(2, false, true));

  PlayerView view{engine, 0, actions};
  EXPECT_TRUE(view.is_available(1));
  EXPECT_FALSE(view.is_available(2));
  EXPECT_FALSE(view.is_available(99));  // unknown verb
}

TEST_F(PlayerViewTest, IsLegalReflectsRuleAndUnknownIsFalse) {
  actions_t actions;
  actions.emplace(1, std::make_unique<FixedAction>(1, true, true));
  actions.emplace(2, std::make_unique<FixedAction>(2, true, false));

  PlayerView view{engine, 0, actions};
  EXPECT_TRUE(view.is_legal(Move{1}));
  EXPECT_FALSE(view.is_legal(Move{2}));
  EXPECT_FALSE(view.is_legal(Move{99}));  // unknown verb
}

}  // namespace