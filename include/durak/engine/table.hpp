#pragma once

#include <optional>
#include <vector>

#include "card.hpp"

namespace durak::engine {

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

}  // namespace durak::engine