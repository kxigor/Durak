#include <utility>

#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>

namespace durak::engine {

GameRuleView::GameRuleView(DurakEngine& engine) : engine_{engine} {}

std::size_t GameRuleView::get_player_count() const {
  return engine_.players_.size();
}

std::size_t GameRuleView::get_player_card_count(modulo_base_t pos) const {
  return engine_.players_.at(static_cast<std::size_t>(pos)).get_card_count();
}

modulo_base_t GameRuleView::get_attacker_pos() const {
  return engine_.attacker_pos_.get_num();
}

modulo_base_t GameRuleView::get_defender_pos() const {
  return engine_.defender_pos_.get_num();
}

modulo_base_t GameRuleView::get_next_offset() const {
  return engine_.k_next_offset_;
}

std::size_t GameRuleView::get_stock_size() const {
  return engine_.stock_.size();
}

std::size_t GameRuleView::get_bita_size() const {
  return engine_.bita_.size();
}

Card GameRuleView::get_trump() const { return engine_.trump_; }

std::optional<player_id_t> GameRuleView::get_last_durak() const {
  if (!engine_.last_game_result_) return std::nullopt;
  return engine_.last_game_result_->durak;
}

void GameRuleView::set_attacker_pos(modulo_base_t pos) {
  auto count = static_cast<modulo_base_t>(engine_.players_.size());
  engine_.attacker_pos_ = DurakEngine::modulo_t{count, pos};
}

void GameRuleView::set_defender_pos(modulo_base_t pos) {
  auto count = static_cast<modulo_base_t>(engine_.players_.size());
  engine_.defender_pos_ = DurakEngine::modulo_t{count, pos};
}

void GameRuleView::set_trump(Card card) { engine_.trump_ = card; }

void GameRuleView::set_stock(std::vector<Card> stock) {
  engine_.stock_ = std::move(stock);
}

std::optional<Card> GameRuleView::draw_from_stock() {
  if (engine_.stock_.empty()) return std::nullopt;
  Card card = engine_.stock_.back();
  engine_.stock_.pop_back();
  return card;
}

void GameRuleView::deal_card(modulo_base_t pos, Card card) {
  engine_.players_.at(static_cast<std::size_t>(pos)).add_card(card);
}

void GameRuleView::add_to_bita(Card card) { engine_.bita_.push_back(card); }

}  // namespace durak::engine
