#pragma once

#include <libpq-fe.h>

#include <initializer_list>
#include <memory>

using QueryResult = std::unique_ptr<PGresult, decltype(&PQclear)>;

class Database {
 public:
  Database();
  QueryResult query(const char* sql,
                    std::initializer_list<const char*> parameters = {},
                    ExecStatusType expected = PGRES_TUPLES_OK);

 private:
  std::unique_ptr<PGconn, decltype(&PQfinish)> db_;
};
