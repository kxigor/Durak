#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>

namespace durak::engine {

GameRuleView::GameRuleView(DurakEngine& engine) : engine_{engine} {}

std::vector<Player>& GameRuleView::players() { return engine_.players_; }

const std::vector<Player>& GameRuleView::players() const {
  return engine_.players_;
}

Table& GameRuleView::table() { return engine_.table_; }

const Table& GameRuleView::table() const { return engine_.table_; }

std::vector<Card>& GameRuleView::stock() { return engine_.stock_; }

const std::vector<Card>& GameRuleView::stock() const { return engine_.stock_; }

std::vector<Card>& GameRuleView::bita() { return engine_.bita_; }

const std::vector<Card>& GameRuleView::bita() const { return engine_.bita_; }

Card& GameRuleView::trump() { return engine_.trump_; }

const Card& GameRuleView::trump() const { return engine_.trump_; }

modulo_base_t GameRuleView::get_attacker_pos() const {
  return engine_.attacker_pos_.get_num();
}

modulo_base_t GameRuleView::get_defender_pos() const {
  return engine_.defender_pos_.get_num();
}

void GameRuleView::set_attacker_pos(modulo_base_t pos) {
  auto count = static_cast<modulo_base_t>(engine_.players_.size());
  engine_.attacker_pos_ = DurakEngine::modulo_t{count, pos};
}

void GameRuleView::set_defender_pos(modulo_base_t pos) {
  auto count = static_cast<modulo_base_t>(engine_.players_.size());
  engine_.defender_pos_ = DurakEngine::modulo_t{count, pos};
}

std::optional<player_id_t> GameRuleView::get_durak() const {
  if (!engine_.last_game_result_) return std::nullopt;
  return engine_.last_game_result_->durak;
}

}  // namespace durak::engine