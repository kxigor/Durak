#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "card.hpp"
#include "config.hpp"
#include "engine_fwd.hpp"

namespace durak::engine {

// Read-write facade handed to rules. Exposes game state, hides control flow
// (start/end/phases) and configuration (the rule set, ring direction).
class GameRuleView {
 public:
  /*================= Constructors/Destructors =================*/
  explicit GameRuleView(DurakEngine& engine);

  /*========================= Getters ==========================*/
  std::size_t get_player_count() const;
  std::size_t get_player_card_count(modulo_base_t pos) const;
  modulo_base_t get_attacker_pos() const;
  modulo_base_t get_defender_pos() const;
  modulo_base_t get_next_offset() const;
  std::size_t get_stock_size() const;
  std::size_t get_bita_size() const;
  Card get_trump() const;
  std::optional<player_id_t> get_last_durak() const;

  /*========================= Setters ==========================*/
  // Positions are set freely (not just advanced): transfers and passing
  // variants move the defender independently of the attacker.
  void set_attacker_pos(modulo_base_t pos);
  void set_defender_pos(modulo_base_t pos);
  void set_trump(Card card);
  void set_stock(std::vector<Card> stock);

  /*======================== Modifiers =========================*/
  std::optional<Card> draw_from_stock();
  void deal_card(modulo_base_t pos, Card card);
  void add_to_bita(Card card);

 private:
  /*======================= Data fields ========================*/
  DurakEngine& engine_;
};

}  // namespace durak::engine
