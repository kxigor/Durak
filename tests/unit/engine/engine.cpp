#include <gtest/gtest.h>

#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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
    for (auto& player : view.players()) player->add_card(Card{1});
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
    while (first->has_cards()) first->remove_card(0);
  }
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 2; }
};

// Round start: seat 1 defends.
struct SetDefenderRule : GameRule {
  void apply(GameRuleView& view) override { view.set_defender_pos(1); }
  bool is_applicable(const GameRuleView& /*view*/) const override {
    return true;
  }
  rule_id_t get_id() const noexcept override { return 3; }
};

std::unique_ptr<GameRule> null_rule() { return std::make_unique<NullRule>(); }

struct StubPlayer : Player {
  using Player::Player;
  move_t get_action(const PlayerView& /*view*/) const override {
    return nullptr;
  }
};

struct ScriptedPlayer : Player {
  action_id_t move_id_;
  mutable bool used_ = false;
  ScriptedPlayer(player_id_t id, action_id_t move_id)
      : Player{id}, move_id_{move_id} {}
  move_t get_action(const PlayerView& /*view*/) const override {
    if (used_) return nullptr;
    used_ = true;
    return std::make_unique<Move>(move_id_);
  }
};

// Plays its verb once, as soon as the current phase offers it.
struct PhasePlayer : Player {
  action_id_t verb_;
  mutable bool used_ = false;
  PhasePlayer(player_id_t id, action_id_t verb) : Player{id}, verb_{verb} {}
  move_t get_action(const PlayerView& view) const override {
    if (used_ || !view.is_available(verb_)) {
      return nullptr;
    }
    used_ = true;
    return std::make_unique<Move>(verb_);
  }
};

struct RecordingAction : ActionRule {
  action_id_t id_;
  int calls_ = 0;
  player_id_t last_player_ = 0;
  action_id_t last_move_id_ = 0;
  explicit RecordingAction(action_id_t id) : id_{id} {}
  bool is_available(player_id_t /*id*/,
                    const GameRuleView& /*view*/) const override {
    return true;
  }
  bool is_legal(player_id_t /*id*/, const Move& /*move*/,
                const GameRuleView& /*view*/) const override {
    return true;
  }
  void apply(player_id_t id, const Move& move,
             GameRuleView& /*view*/) override {
    ++calls_;
    last_player_ = id;
    last_move_id_ = move.get_action_id();
  }
  action_id_t get_id() const noexcept override { return id_; }
};

struct IllegalAction : ActionRule {
  action_id_t id_;
  bool applied_ = false;
  explicit IllegalAction(action_id_t id) : id_{id} {}
  bool is_available(player_id_t /*id*/,
                    const GameRuleView& /*view*/) const override {
    return true;
  }
  bool is_legal(player_id_t /*id*/, const Move& /*move*/,
                const GameRuleView& /*view*/) const override {
    return false;
  }
  void apply(player_id_t /*id*/, const Move& /*move*/,
             GameRuleView& /*view*/) override {
    applied_ = true;
  }
  action_id_t get_id() const noexcept override { return id_; }
};

struct ThrowingAction : ActionRule {
  action_id_t id_;
  explicit ThrowingAction(action_id_t id) : id_{id} {}
  bool is_available(player_id_t /*id*/,
                    const GameRuleView& /*view*/) const override {
    return true;
  }
  bool is_legal(player_id_t /*id*/, const Move& /*move*/,
                const GameRuleView& /*view*/) const override {
    return true;
  }
  void apply(player_id_t /*id*/, const Move& /*move*/,
             GameRuleView& /*view*/) override {
    throw std::logic_error("boom");
  }
  action_id_t get_id() const noexcept override { return id_; }
};

players_t make_players(std::initializer_list<player_id_t> ids) {
  players_t players;
  for (auto id : ids) players.push_back(std::make_unique<StubPlayer>(id));
  return players;
}

// --- Lifetime ---

TEST(EngineTest, StartMarksGameRunning) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  engine.start(make_players({1, 2}));
  EXPECT_TRUE(engine.get_status().is_game_running);
}

TEST(EngineTest, EndStopsGame) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  engine.start(make_players({1, 2}));
  engine.end();
  EXPECT_FALSE(engine.get_status().is_game_running);
}

// --- Full game to termination ---

TEST(EngineTest, GameEndsWithLastHolderAsDurak) {
  DurakEngine engine{null_rule(),
                     1,
                     std::make_unique<DealOneRule>(),  // deck init: deal cards
                     null_rule(),                      // round start
                     std::make_unique<ClearFirstRule>(),  // round end: empty p0
                     {},
                     {},
                     {}};
  engine.start(make_players({10, 20}));

  auto result = engine.get_last_game_result();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->durak, player_id_t{20});
}

// --- Toss/beat phases ---

TEST(EngineTest, TossAndBeatPhasesDeliverMoves) {
  auto toss_action = std::make_unique<RecordingAction>(21);
  auto beat_action = std::make_unique<RecordingAction>(22);
  auto* toss_rec = toss_action.get();
  auto* beat_rec = beat_action.get();
  actions_t toss_actions;
  toss_actions.emplace(21, std::move(toss_action));
  actions_t beat_actions;
  beat_actions.emplace(22, std::move(beat_action));

  DurakEngine engine{null_rule(),
                     1,
                     std::make_unique<DealOneRule>(),
                     std::make_unique<SetDefenderRule>(),
                     std::make_unique<ClearFirstRule>(),
                     {},
                     std::move(toss_actions),
                     std::move(beat_actions)};

  players_t players;
  players.push_back(std::make_unique<PhasePlayer>(10, 21));  // attacker tosses
  players.push_back(std::make_unique<PhasePlayer>(20, 22));  // defender beats
  engine.start(std::move(players));

  EXPECT_EQ(toss_rec->calls_, 1);
  EXPECT_EQ(toss_rec->last_player_, player_id_t{10});
  EXPECT_EQ(beat_rec->calls_, 1);
  EXPECT_EQ(beat_rec->last_player_, player_id_t{20});

  auto result = engine.get_last_game_result();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->durak, player_id_t{20});
}

// --- Constructor validation ---

TEST(EngineTest, CtorRejectsNullRules) {
  EXPECT_THROW(
      (DurakEngine{
          nullptr, 1, null_rule(), null_rule(), null_rule(), {}, {}, {}}),
      std::invalid_argument);
  EXPECT_THROW(
      (DurakEngine{
          null_rule(), 1, nullptr, null_rule(), null_rule(), {}, {}, {}}),
      std::invalid_argument);
  EXPECT_THROW(
      (DurakEngine{
          null_rule(), 1, null_rule(), nullptr, null_rule(), {}, {}, {}}),
      std::invalid_argument);
  EXPECT_THROW(
      (DurakEngine{
          null_rule(), 1, null_rule(), null_rule(), nullptr, {}, {}, {}}),
      std::invalid_argument);
}

// --- Offset validation ---

TEST(EngineTest, StartRejectsNonCoprimeOffset) {
  DurakEngine engine{null_rule(), 2,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  EXPECT_THROW(engine.start(make_players({1, 2})), std::invalid_argument);
  EXPECT_FALSE(engine.get_status().is_game_running);  // engine untouched
  EXPECT_FALSE(engine.get_last_game_result().has_value());
}

// --- Session reset ---

TEST(EngineTest, StartResetsSessionState) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};
  GameRuleView dirty{engine};
  dirty.stock().push_back(Card{1});
  dirty.bita().push_back(Card{2});
  dirty.trump() = Card{3};
  dirty.table().add_attack(Card{4});
  med.set_last_durak(player_id_t{5});

  engine.start(make_players({1, 2}));

  EXPECT_TRUE(med.stock().empty());
  EXPECT_TRUE(med.bita().empty());
  EXPECT_EQ(med.trump().id, 0);
  EXPECT_EQ(dirty.table().size(), 0u);
  ASSERT_TRUE(med.has_result());                 // fresh result of this game
  EXPECT_FALSE(med.result_durak().has_value());  // stale durak wiped
}

// --- is_game_over ---

TEST(EngineTest, GameOverWhenAtMostOneHolder) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  med.players().push_back(std::make_unique<StubPlayer>(1));
  med.players().push_back(std::make_unique<StubPlayer>(2));
  med.players()[0]->add_card(Card{1});
  EXPECT_TRUE(med.is_game_over());  // only one holder

  med.players()[1]->add_card(Card{2});
  EXPECT_FALSE(med.is_game_over());  // two holders
}

// --- define_loser ---

TEST(EngineTest, DefineLoserPicksTheHolder) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  med.players().push_back(std::make_unique<StubPlayer>(1));
  med.players().push_back(std::make_unique<StubPlayer>(2));
  med.players()[1]->add_card(Card{1});

  med.define_loser();
  ASSERT_TRUE(med.has_result());
  EXPECT_EQ(med.result_durak(), player_id_t{2});
}

TEST(EngineTest, DefineLoserNoDurakWhenAllEmpty) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  med.players().push_back(std::make_unique<StubPlayer>(1));
  med.players().push_back(std::make_unique<StubPlayer>(2));

  med.define_loser();
  ASSERT_TRUE(med.has_result());
  EXPECT_FALSE(med.result_durak().has_value());  // a draw, no durak
}

// --- Positions ---

TEST(EngineTest, ReportsSeatedPositions) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};
  med.players().push_back(std::make_unique<StubPlayer>(1));
  med.players().push_back(std::make_unique<StubPlayer>(2));
  med.players().push_back(std::make_unique<StubPlayer>(3));
  med.seat_positions(0, 1);

  EXPECT_EQ(engine.get_attacker_pos(), 0);
  EXPECT_EQ(engine.get_defender_pos(), 1);
  EXPECT_EQ(med.attacker_id(), player_id_t{1});
  EXPECT_EQ(med.defender_id(), player_id_t{2});
}

// --- Player cycle / action routing ---

TEST(EngineTest, CyclePlayerRoutesMoveToMatchingAction) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  auto action = std::make_unique<RecordingAction>(7);
  auto* recorded = action.get();
  actions_t actions;
  actions.emplace(7, std::move(action));

  ScriptedPlayer player{10, 7};
  EXPECT_TRUE(med.cycle_player(player, actions));  // player acted

  EXPECT_EQ(recorded->calls_, 1);
  EXPECT_EQ(recorded->last_player_, player_id_t{10});
  EXPECT_EQ(recorded->last_move_id_, action_id_t{7});
}

TEST(EngineTest, CyclePlayerReportsNoActionWhenPlayerDeclines) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  actions_t none;
  StubPlayer player{10};
  EXPECT_FALSE(med.cycle_player(player, none));  // never acted
}

TEST(EngineTest, ApplyActionRejectsUnknownVerb) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  actions_t none;
  Move move{99};
  EXPECT_THROW(med.apply_action(1, move, none), std::out_of_range);
}

TEST(EngineTest, ApplyActionRejectsIllegalMove) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  auto action = std::make_unique<IllegalAction>(3);
  auto* rejected = action.get();
  actions_t actions;
  actions.emplace(3, std::move(action));

  Move move{3};
  EXPECT_THROW(med.apply_action(1, move, actions), std::invalid_argument);
  EXPECT_FALSE(rejected->applied_);  // guard blocked apply
}

TEST(EngineTest, ApplyActionPropagatesRuleException) {
  DurakEngine engine{null_rule(), 1,  null_rule(), null_rule(),
                     null_rule(), {}, {},          {}};
  DurakEngineTestMediator med{engine};

  actions_t actions;
  actions.emplace(3, std::make_unique<ThrowingAction>(3));
  Move move{3};
  EXPECT_THROW(med.apply_action(1, move, actions), std::logic_error);
}

}  // namespace