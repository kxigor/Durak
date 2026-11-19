#include "scores.hpp"

void Scores::record(const std::string& game, const std::string& owner,
                    const std::string& outcome) {
  static constexpr char kInsert[] =
      "INSERT INTO results (id, name, outcome, owner) VALUES ($1, $2, $3, $2) "
      "ON CONFLICT (id) DO NOTHING";
  db_.query(kInsert, {game.c_str(), owner.c_str(), outcome.c_str()},
            PGRES_COMMAND_OK);
}

nlohmann::json Scores::leaders() {
  static constexpr char kQuery[] =
      "SELECT name, count(*) FILTER (WHERE outcome='win') AS wins, "
      "count(*) FILTER (WHERE outcome='loss') AS losses, "
      "count(*) FILTER (WHERE outcome='draw') AS draws, registered FROM "
      "(SELECT owner AS name, outcome, true AS registered FROM results "
      "WHERE owner IS NOT NULL "
      "UNION ALL "
      "SELECT login AS name, NULL AS outcome, false AS registered FROM users) "
      "AS entries "
      "GROUP BY name, registered ORDER BY wins DESC, losses ASC, draws DESC, "
      "name, registered DESC LIMIT 12";
  auto result = db_.query(kQuery);
  auto rows = nlohmann::json::array();
  for (int row = 0; row < PQntuples(result.get()); ++row) {
    rows.push_back({{"name", PQgetvalue(result.get(), row, 0)},
                    {"wins", std::stoll(PQgetvalue(result.get(), row, 1))},
                    {"losses", std::stoll(PQgetvalue(result.get(), row, 2))},
                    {"draws", std::stoll(PQgetvalue(result.get(), row, 3))}});
    if (std::string_view{PQgetvalue(result.get(), row, 4)} == "t")
      rows.back()["registered"] = true;
  }
  return rows;
}
