#pragma once

#include <array>
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

  static constexpr std::size_t kMaxPlayers = 6;

  explicit Match(std::mt19937::result_type seed = std::random_device{}());
  Match(std::size_t players, std::mt19937::result_type seed);
  void play(int move);
  [[nodiscard]] std::size_t actor() const;
  [[nodiscard]] int computer_move() const;
  [[nodiscard]] std::vector<int> legal_moves() const;
  [[nodiscard]] const engine::GameRuleView& board() const noexcept;
  [[nodiscard]] Phase phase() const noexcept;
  [[nodiscard]] std::string_view result(std::size_t seat = 0) const;

 private:
  void offer_throws(Phase phase);
  [[nodiscard]] bool accepts(engine::Card card) const;
  void apply(int move);
  void start_round();
  void end_round();
  void play_bot();

  engine::DurakEngine engine_;
  engine::GameRuleView board_;
  Phase phase_{Phase::Attack};
  std::size_t limit_{};
  std::vector<std::size_t> throwers_;
  std::array<bool, kMaxPlayers> went_out_{};
  bool solo_{};
};

}  // namespace durak::rules
