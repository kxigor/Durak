#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>

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
  const std::string& get_name() const noexcept override {
    static const std::string name{"null"};
    return name;
  }
};

std::unique_ptr<GameRule> null_rule() { return std::make_unique<NullRule>(); }

struct GameRuleViewTest : ::testing::Test {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  GameRuleView view{engine};
};

// --- Stock ---

TEST_F(GameRuleViewTest, StockRoundTrip) {
  view.stock().push_back(Card{3});
  view.stock().push_back(Card{4});
  const GameRuleView& cview = view;
  EXPECT_EQ(cview.stock().size(), 2u);
  EXPECT_EQ(cview.stock()[0].id, 3);
}

// --- Bita ---

TEST_F(GameRuleViewTest, BitaRoundTrip) {
  view.bita().push_back(Card{9});
  const GameRuleView& cview = view;
  EXPECT_EQ(cview.bita().size(), 1u);
  EXPECT_EQ(cview.bita()[0].id, 9);
}

// --- Trump ---

TEST_F(GameRuleViewTest, TrumpRoundTrip) {
  view.trump() = Card{11};
  const GameRuleView& cview = view;
  EXPECT_EQ(cview.trump().id, 11);
}

// --- Table ---

TEST_F(GameRuleViewTest, TableRoundTrip) {
  view.table().add_attack(Card{2});
  const GameRuleView& cview = view;
  EXPECT_EQ(cview.table().size(), 1u);
  EXPECT_EQ(cview.table().get_cards()[0].get_attack_card().id, 2);
}

// --- Players ---

TEST_F(GameRuleViewTest, PlayersRoundTrip) {
  view.players().push_back(Player{7});
  const GameRuleView& cview = view;
  EXPECT_EQ(cview.players().size(), 1u);
  EXPECT_EQ(cview.players()[0].get_id(), 7u);
}

// --- Positions ---

TEST_F(GameRuleViewTest, PositionsRoundTrip) {
  view.players().push_back(Player{1});
  view.players().push_back(Player{2});
  view.players().push_back(Player{3});
  view.set_attacker_pos(1);
  view.set_defender_pos(2);
  EXPECT_EQ(view.get_attacker_pos(), 1);
  EXPECT_EQ(view.get_defender_pos(), 2);
}

// --- Durak result ---

TEST_F(GameRuleViewTest, DurakEmptyByDefault) {
  EXPECT_FALSE(view.get_durak().has_value());
}

TEST_F(GameRuleViewTest, DurakReportsLastLoser) {
  DurakEngineTestMediator med{engine};
  med.set_last_durak(player_id_t{5});
  EXPECT_EQ(view.get_durak(), player_id_t{5});
}

}  // namespace