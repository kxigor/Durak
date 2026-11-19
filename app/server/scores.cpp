#include "scores.hpp"

#include <array>
#include <stdexcept>

namespace {
using Result = std::unique_ptr<PGresult, decltype(&PQclear)>;

Result checked(PGresult* raw, ExecStatusType expected) {
  Result result{raw, PQclear};
  if (!result || PQresultStatus(result.get()) != expected)
    throw std::runtime_error{"leaderboard query failed"};
  return result;
}
}  // namespace

Scores::Scores() : db_{PQconnectdb(""), PQfinish} {
  if (!db_ || PQstatus(db_.get()) != CONNECTION_OK)
    throw std::runtime_error{"cannot connect to leaderboard"};
}

PGconn* Scores::connection() {
  if (PQstatus(db_.get()) != CONNECTION_OK) PQreset(db_.get());
  return db_.get();
}

void Scores::record(const std::string& game, const std::string& name,
                    const std::string& outcome) {
  constexpr auto kInsert =
      "INSERT INTO results (id, name, outcome) VALUES ($1, $2, $3) "
      "ON CONFLICT (id) DO NOTHING";
  const std::array params{game.c_str(), name.c_str(), outcome.c_str()};
  checked(PQexecParams(connection(), kInsert, 3, nullptr, params.data(),
                       nullptr, nullptr, 0),
          PGRES_COMMAND_OK);
}

nlohmann::json Scores::leaders() {
  constexpr auto kQuery =
      "SELECT name, count(*) FILTER (WHERE outcome='win') AS wins, "
      "count(*) FILTER (WHERE outcome='loss') AS losses, "
      "count(*) FILTER (WHERE outcome='draw') AS draws FROM "
      "(SELECT name, outcome FROM results UNION ALL "
      "SELECT login AS name, NULL AS outcome FROM users) AS entries "
      "GROUP BY name ORDER BY wins DESC, losses ASC, draws DESC, name LIMIT 12";
  auto result = checked(PQexec(connection(), kQuery), PGRES_TUPLES_OK);
  auto rows = nlohmann::json::array();
  for (int row = 0; row < PQntuples(result.get()); ++row) {
    rows.push_back({{"name", PQgetvalue(result.get(), row, 0)},
                    {"wins", std::stoll(PQgetvalue(result.get(), row, 1))},
                    {"losses", std::stoll(PQgetvalue(result.get(), row, 2))},
                    {"draws", std::stoll(PQgetvalue(result.get(), row, 3))}});
  }
  return rows;
}
