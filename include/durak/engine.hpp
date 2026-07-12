#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "modulo.hpp"

namespace durak::engine {

class DurakEngine;

struct Card {
  using card_id_t = std::uint16_t;

  card_id_t id{};
};

class Rule {
 public:
  /*======================= Destructors ========================*/
  virtual ~Rule() = default;

  /*========================= Game API =========================*/
  virtual void apply(DurakEngine& engine) = 0;
  virtual bool is_applicable(DurakEngine& engine) const = 0;
  virtual const std::string& get_name() const noexcept = 0;
};

class CardOnTable {
 public:
  /*================= Constructors/Destructors =================*/
  CardOnTable(Card attack_card);

  CardOnTable(const CardOnTable& /*unused*/) = delete;
  CardOnTable(CardOnTable&& /*unused*/) = default;

  ~CardOnTable() = default;

  /*======================= Assignments ========================*/
  CardOnTable& operator=(const CardOnTable& /*unused*/) = delete;
  CardOnTable& operator=(CardOnTable&& /*unused*/) = default;

  /*======================== Modifiers =========================*/
  void beat(Card beat_card);

  /*========================= Getters ==========================*/
  Card get_attack_card() const;

  Card get_beat_card() const;

  bool is_beaten() const;

 private:
  /*======================= Data fields ========================*/
  Card attack_card_{};
  std::optional<Card> beat_card_{};
};

class Table {
 public:
  /*====================== Usings/Helpers ======================*/
  using cards_on_table_t = std::vector<CardOnTable>;

  /*================= Constructors/Destructors =================*/
  Table() = default;

  Table(const Table& /*unused*/) = delete;
  Table(Table&& /*unused*/) = delete;

  ~Table() = default;

  /*======================= Assignments ========================*/
  Table& operator=(const Table& /*unused*/) = delete;
  Table& operator=(Table&& /*unused*/) = delete;

  /*======================== Modifiers =========================*/
  void clear();

  /*========================= Getters ==========================*/
  const cards_on_table_t& get_cards() const;

 private:
  /*======================= Data fields ========================*/
  cards_on_table_t cards_{};
};

class Player {
 public:
  /*====================== Usings/Helpers ======================*/
  using player_id_t = std::uint64_t;

  /*================= Constructors/Destructors =================*/
  Player(const Player& /*unused*/) = delete;
  Player(Player&& /*unused*/) = default;

  ~Player() = default;

  /*======================= Assignments ========================*/
  Player& operator=(const Player& /*unused*/) = delete;
  Player& operator=(Player&& /*unused*/) = default;

 private:
  /*======================= Data fields ========================*/
  std::vector<Card> cards_{};
};

class DurakEngine {
  /*====================== Usings/Helpers ======================*/
  using modulo_base_t = std::int8_t;
  using modulo_t = modulo::ModuloRing<modulo_base_t>;

  using rule_t = std::unique_ptr<Rule>;
  using rules_t = std::vector<rule_t>;

  struct GameStatus {
    bool is_game_running_{false};
  };

  struct GameResult {
    std::optional<Player::player_id_t> durak{};
  };

 public:
  /*================= Constructors/Destructors =================*/
  DurakEngine(rule_t first_attacker, modulo_base_t next_offset,
              rule_t init_stock, rules_t leading_rules, rules_t attacking_rules,
              rules_t defending_rules);

  DurakEngine(const DurakEngine& /*unused*/) = delete;
  DurakEngine(DurakEngine&& /*unused*/) = delete;

  ~DurakEngine() = default;

  /*======================= Assignments ========================*/
  DurakEngine& operator=(const DurakEngine& /*unused*/) = delete;
  DurakEngine& operator=(DurakEngine&& /*unused*/) = delete;

  /*====================== Game Lifetime =======================*/
  void start(std::vector<Player> players);

  void end();

  /*========================= Game API =========================*/
  const GameStatus& get_status() const;

  modulo_base_t get_attacking_player_pos() const;
  modulo_base_t get_defending_player_pos() const;
  std::optional<GameResult> get_last_game_result() const;

 private:
  /*======================= Data fields ========================*/
  /*======================== Game Rules ========================*/
  const rule_t k_first_attacker_rule_{};
  const modulo_base_t k_next_offset_{};
  const rule_t k_init_stock_rule_{};
  const rules_t k_leading_rules_{};
  const rules_t k_attacking_rules_{};
  const rules_t k_defending_rules_{};

  /*======================= Game Process =======================*/
  GameStatus game_status_{};

  std::vector<Player> players_{};

  Table table_{};

  modulo_t attacking_pos_{};
  modulo_t defending_pos_{};

  std::vector<Card> stock_{};
  Card trump_{};

  std::optional<GameResult> last_game_result_{};
};

};  // namespace durak::engine