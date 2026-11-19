#pragma once

#include <libpq-fe.h>

#include <cstddef>
#include <initializer_list>
#include <memory>

using QueryResult = std::unique_ptr<PGresult, decltype(&PQclear)>;

struct SqlStatement {
  template <std::size_t N>
  consteval SqlStatement(const char (&literal)[N]) : text{literal} {}

  const char* const text;
};

class Database {
 public:
  Database();
  QueryResult query(SqlStatement sql,
                    std::initializer_list<const char*> parameters = {},
                    ExecStatusType expected = PGRES_TUPLES_OK);

 private:
  std::unique_ptr<PGconn, decltype(&PQfinish)> db_;
};
