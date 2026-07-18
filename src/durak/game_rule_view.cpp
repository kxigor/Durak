#include <include/durak/engine/card.hpp>
#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine.hpp>
#include <include/durak/engine/game_rule_view.hpp>
#include <include/durak/engine/player.hpp>
#include <include/durak/engine/table.hpp>
#include <optional>
#include <vector>

namespace durak::engine {

GameRuleView::GameRuleView(DurakEngine& engine) noexcept : engine_{engine} {}

players_t& GameRuleView::players() noexcept { return engine_.players_; }

const players_t& GameRuleView::players() const noexcept {
  return engine_.players_;
}

Table& GameRuleView::table() noexcept { return engine_.table_; }

const Table& GameRuleView::table() const noexcept { return engine_.table_; }

std::vector<Card>& GameRuleView::stock() noexcept { return engine_.stock_; }

const std::vector<Card>& GameRuleView::stock() const noexcept {
  return engine_.stock_;
}

std::vector<Card>& GameRuleView::bita() noexcept { return engine_.bita_; }

const std::vector<Card>& GameRuleView::bita() const noexcept {
  return engine_.bita_;
}

Card& GameRuleView::trump() noexcept { return engine_.trump_; }

const Card& GameRuleView::trump() const noexcept { return engine_.trump_; }

modulo_base_t GameRuleView::get_attacker_pos() const noexcept {
  return engine_.attacker_pos_.get_num();
}

modulo_base_t GameRuleView::get_defender_pos() const noexcept {
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

std::optional<player_id_t> GameRuleView::get_durak() const noexcept {
  if (!engine_.last_game_result_) {
    return std::nullopt;
  }
  return engine_.last_game_result_->durak;
}

}  // namespace durak::engine
