#pragma once

#include <cstddef>
#include <cstdint>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <random>
#include <string_view>
#include <vector>

namespace durak::rules {

enum class Phase : std::uint8_t { Attack, Defend, Throw, Pickup, Finished };

class Match {
 public:
  static constexpr int kTake = -1;
  static constexpr int kDone = -2;

  explicit Match(std::mt19937::result_type seed = std::random_device{}());
  void play(int move);
  [[nodiscard]] std::vector<int> legal_moves() const;
  [[nodiscard]] const engine::GameRuleView& board() const noexcept;
  [[nodiscard]] Phase phase() const noexcept;
  [[nodiscard]] std::string_view result() const noexcept;

 private:
  [[nodiscard]] std::size_t actor() const;
  [[nodiscard]] bool accepts(engine::Card card) const;
  void apply(int move);
  void start_round();
  void end_round();
  void play_bot();

  engine::DurakEngine engine_;
  engine::GameRuleView board_;
  Phase phase_{Phase::Attack};
  std::size_t limit_{};
};

}  // namespace durak::rules
