#include <algorithm>
#include <cstddef>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/move.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/player_view.hpp>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/deck_init.hpp>
#include <include/durak/rules/first_turn.hpp>
#include <include/durak/rules/game_constants.hpp>
#include <include/durak/rules/match.hpp>
#include <include/durak/rules/round_end.hpp>
#include <include/durak/rules/round_start.hpp>
#include <memory>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace durak::rules {
namespace {
class Seat final : public engine::Player {
 public:
  using Player::Player;
  [[nodiscard]] engine::move_t get_action(
      const engine::PlayerView& /*view*/) const override {
    return nullptr;
  }
};

}  // namespace

Match::Match(std::mt19937::result_type seed)
    : engine_{FirstTurnRule::make(),
              1,
              DeckInitRule::make(deck::kThirtySix, seed),
              RoundStartRule::make(),
              RoundEndRule::make(),
              {},
              {},
              {}},
      board_{engine_} {
  engine::players_t seats;
  seats.push_back(std::make_unique<Seat>(0U));
  seats.push_back(std::make_unique<Seat>(1U));
  engine_.prepare(std::move(seats));
  start_round();
  play_bot();
}

const engine::GameRuleView& Match::board() const noexcept { return board_; }
Phase Match::phase() const noexcept { return phase_; }

std::string_view Match::result() const noexcept {
  if (phase_ != Phase::Finished) {
    return {};
  }
  if (board_.players().at(0)->has_cards()) {
    return "loss";
  }
  return board_.players().at(1)->has_cards() ? "win" : "draw";
}

std::size_t Match::actor() const {
  return static_cast<std::size_t>(phase_ == Phase::Defend
                                      ? board_.get_defender_pos()
                                      : board_.get_attacker_pos());
}

bool Match::accepts(engine::Card card) const {
  const auto kCandidate = CardFace::from_id(card.id);
  const auto& table = board_.table().get_cards();
  if (phase_ == Phase::Defend) {
    const auto kAttack = CardFace::from_id(table.back().get_attack_card().id);
    if (kCandidate.suit == kAttack.suit) {
      return kCandidate.rank > kAttack.rank;
    }
    return kCandidate.suit == CardFace::from_id(board_.trump().id).suit;
  }
  if (table.size() >= limit_) {
    return false;
  }
  if (table.empty()) {
    return true;
  }
  return std::ranges::any_of(table, [&](const auto& pair) {
    return CardFace::from_id(pair.get_attack_card().id).rank ==
               kCandidate.rank ||
           (pair.is_beaten() &&
            CardFace::from_id(pair.get_beat_card().id).rank == kCandidate.rank);
  });
}

std::vector<int> Match::legal_moves() const {
  std::vector<int> moves;
  if (phase_ == Phase::Finished) {
    return moves;
  }
  for (auto card : board_.players().at(actor())->get_cards()) {
    if (accepts(card)) {
      moves.push_back(card.id);
    }
  }
  if (phase_ == Phase::Defend) {
    moves.push_back(kTake);
  }
  if (phase_ == Phase::Throw || phase_ == Phase::Pickup) {
    moves.push_back(kDone);
  }
  return moves;
}

void Match::start_round() {
  RoundStartRule{}.apply(board_);
  const auto kDefender = static_cast<unsigned char>(board_.get_defender_pos());
  limit_ =
      std::min(kHandSize, board_.players().at(kDefender)->get_card_count());
  phase_ = Phase::Attack;
}

void Match::end_round() {
  RoundEndRule{}.apply(board_);
  if (board_.stock().empty() && (!board_.players().at(0)->has_cards() ||
                                 !board_.players().at(1)->has_cards())) {
    phase_ = Phase::Finished;
    engine_.end();
    return;
  }
  start_round();
}

void Match::apply(int move) {
  if (move == kDone) {
    end_round();
    return;
  }
  if (move == kTake) {
    phase_ = Phase::Pickup;
    return;
  }
  auto& player = *board_.players().at(actor());
  const auto& hand = player.get_cards();
  const auto kPos = std::ranges::find(hand, move, &engine::Card::id);
  const auto kCard =
      player.remove_card(static_cast<std::size_t>(kPos - hand.begin()));
  auto& table = board_.table();
  if (phase_ == Phase::Defend) {
    table.beat(table.size() - 1, kCard);
    phase_ = Phase::Throw;
  } else {
    table.add_attack(kCard);
    if (phase_ != Phase::Pickup) {
      phase_ = Phase::Defend;
    }
  }
}

void Match::play_bot() {
  while (phase_ != Phase::Finished && actor() == 1) {
    const auto kMoves = legal_moves();
    const auto kTrump = CardFace::from_id(board_.trump().id).suit;
    constexpr int kTrumpPenalty = 20;
    constexpr int kPassCost = 100;
    const auto kCost = [kTrump](int move) {
      if (move < 0) {
        return kPassCost;
      }
      const auto kCard =
          CardFace::from_id(static_cast<engine::card_id_t>(move));
      return static_cast<int>(kCard.rank) +
             (kCard.suit == kTrump ? kTrumpPenalty : 0);
    };
    apply(*std::ranges::min_element(kMoves, {}, kCost));
  }
}

void Match::play(int move) {
  const auto kMoves = legal_moves();
  if (std::ranges::find(kMoves, move) == kMoves.end()) {
    throw std::invalid_argument{"illegal move"};
  }
  apply(move);
  play_bot();
}

}  // namespace durak::rules
