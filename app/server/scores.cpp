#include "scores.hpp"

#include <memory>
#include <stdexcept>

namespace {
using Statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

Statement prepare(sqlite3* db, const char* sql) {
  sqlite3_stmt* raw{};
  if (sqlite3_prepare_v2(db, sql, -1, &raw, nullptr) != SQLITE_OK)
    throw std::runtime_error{"database prepare failed"};
  return Statement{raw, sqlite3_finalize};
}

void bind(sqlite3_stmt* stmt, int index, const std::string& value) {
  if (sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT) !=
      SQLITE_OK)
    throw std::runtime_error{"database bind failed"};
}
}  // namespace

Scores::Scores(const char* path) {
  if (sqlite3_open(path, &db_) != SQLITE_OK ||
      sqlite3_exec(
          db_,
          "PRAGMA max_page_count=65536;"
          "CREATE TABLE IF NOT EXISTS results("
          "id TEXT PRIMARY KEY, name TEXT NOT NULL,"
          "outcome TEXT NOT NULL CHECK(outcome IN ('win','loss','draw')));"
          "CREATE INDEX IF NOT EXISTS names ON results(name);",
          nullptr, nullptr, nullptr) != SQLITE_OK) {
    sqlite3_close(db_);
    throw std::runtime_error{"cannot initialize leaderboard"};
  }
}
Scores::~Scores() { sqlite3_close(db_); }

void Scores::record(const std::string& game, const std::string& name,
                    const std::string& outcome) {
  auto stmt = prepare(
      db_, "INSERT INTO results VALUES (?1,?2,?3) ON CONFLICT(id) DO NOTHING");
  bind(stmt.get(), 1, game);
  bind(stmt.get(), 2, name);
  bind(stmt.get(), 3, outcome);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE)
    throw std::runtime_error{"cannot save result"};
}

nlohmann::json Scores::leaders() const {
  auto rows = nlohmann::json::array();
  auto stmt = prepare(
      db_,
      "SELECT name,SUM(outcome='win'),SUM(outcome='loss'),SUM(outcome='draw') "
      "FROM results GROUP BY name ORDER BY 2 DESC,3 ASC,4 DESC,name LIMIT 12");
  int status{};
  while ((status = sqlite3_step(stmt.get())) == SQLITE_ROW) {
    const auto* name =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
    rows.push_back({{"name", name},
                    {"wins", sqlite3_column_int64(stmt.get(), 1)},
                    {"losses", sqlite3_column_int64(stmt.get(), 2)},
                    {"draws", sqlite3_column_int64(stmt.get(), 3)}});
  }
  if (status != SQLITE_DONE)
    throw std::runtime_error{"cannot read leaderboard"};
  return rows;
}
