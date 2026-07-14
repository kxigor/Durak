#include <include/durak/engine/engine.hpp>

using namespace durak::engine;

struct StubRule : GameRule {
  void apply(GameRuleView&) override {}
  bool is_applicable(const GameRuleView&) const override { return true; }
  rule_id_t get_id() const noexcept override { return 0; }
};

int main() {
  DurakEngine engine{
      std::make_unique<StubRule>(),
      1,
      std::make_unique<StubRule>(),
      std::make_unique<StubRule>(),
      std::make_unique<StubRule>(),
      {},
      {},
      {},
  };

  std::vector<Player> players;
  players.emplace_back(player_id_t{1});
  players.emplace_back(player_id_t{2});

  engine.start(std::move(players));
}
