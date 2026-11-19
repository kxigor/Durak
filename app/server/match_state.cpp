#include "match_state.hpp"

using durak::rules::Match;
using durak::rules::Phase;
using Json = nlohmann::json;
namespace {
std::string_view phase_name(Phase phase) {
  switch (phase) {
    case Phase::Attack:
      return "attack";
    case Phase::Defend:
      return "defend";
    case Phase::Throw:
      return "throw";
    case Phase::Pickup:
      return "pickup";
    case Phase::Finished:
      return "finished";
    default:
      throw std::logic_error{"unknown phase"};
  }
}

}  // namespace

Json match_state(const Match& match, std::size_t seat, const std::string& name,
                 int version) {
  const auto& board = match.board();
  auto hand = Json::array();
  for (const auto card : board.players().at(seat)->get_cards())
    hand.push_back(card.id);
  auto table = Json::array();
  for (const auto& pair : board.table().get_cards()) {
    table.push_back(
        {{"attack", pair.get_attack_card().id},
         {"defense",
          pair.is_beaten() ? Json(pair.get_beat_card().id) : Json(nullptr)}});
  }
  auto result = match.result(seat);
  return {{"name", name},
          {"hand", hand},
          {"table", table},
          {"stock_count", board.stock().size()},
          {"trump", board.trump().id},
          {"phase", phase_name(match.phase())},
          {"result", result.empty() ? Json(nullptr) : Json(result)},
          {"moves", match.phase() != Phase::Finished && match.actor() == seat
                        ? Json(match.legal_moves())
                        : Json::array()},
          {"version", version}};
}
