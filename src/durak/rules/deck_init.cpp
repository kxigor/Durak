#include <algorithm>
#include <cstdint>
#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>
#include <include/durak/engine/game_rule.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/rules/card_face.hpp>
#include <include/durak/rules/deck_init.hpp>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

namespace durak::rules {

DeckInitRule::DeckInitRule(DeckKind kind, std::mt19937::result_type seed)
    : kind_{kind}, gen_{seed} {
  if (!kind_.valid()) {
    throw std::invalid_argument("DeckInitRule: invalid deck kind");
  }
}

engine::rule_t DeckInitRule::make(DeckKind kind,
                                  std::mt19937::result_type seed) {
  return std::make_unique<DeckInitRule>(kind, seed);
}

void DeckInitRule::apply(engine::GameRuleView& view) {
  auto& stock = view.stock();
  auto& bita = view.bita();
  auto& trump = view.trump();

  init_bita(bita);
  init_stock(stock);
  std::shuffle(stock.begin(), stock.end(), gen_);
  init_trump(stock, trump);
}

bool DeckInitRule::is_applicable(const engine::GameRuleView& /*view*/) const {
  return true;
}

engine::rule_id_t DeckInitRule::get_id() const noexcept { return kRuleId; }

void DeckInitRule::init_stock(std::vector<engine::Card>& stock) {
  stock.clear();
  stock.reserve(kind_.size());

  const auto kLo = static_cast<engine::card_id_t>(
      (static_cast<std::uint32_t>(kind_.lowest) - CardFace::kMinRank)
      << CardFace::kRankShift);
  const auto kHi = static_cast<engine::card_id_t>(
      (static_cast<std::uint32_t>(Rank::Ace) - CardFace::kMinRank + 1)
      << CardFace::kRankShift);

  for (engine::card_id_t id = kLo; id < kHi; ++id) {
    stock.emplace_back(id);
  }

  if (kind_.jokers) {
    const auto kJoker = static_cast<engine::card_id_t>(
        (static_cast<std::uint32_t>(Rank::Joker) - CardFace::kMinRank)
        << CardFace::kRankShift);
    stock.emplace_back(kJoker);
    stock.emplace_back(static_cast<engine::card_id_t>(kJoker + 1));
  }
}

void DeckInitRule::init_bita(std::vector<engine::Card>& bita) { bita.clear(); }

void DeckInitRule::init_trump(std::vector<engine::Card>& stock,
                              engine::Card& trump) {
  trump = stock.back();
}

}  // namespace durak::rules
