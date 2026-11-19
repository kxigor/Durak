#include "database.hpp"

#include <stdexcept>

Database::Database() : db_{PQconnectdb(""), PQfinish} {
  if (!db_ || PQstatus(db_.get()) != CONNECTION_OK)
    throw std::runtime_error{"cannot connect to the database"};
}

QueryResult Database::query(SqlStatement sql,
                            std::initializer_list<const char*> parameters,
                            ExecStatusType expected) {
  if (PQstatus(db_.get()) != CONNECTION_OK) PQreset(db_.get());
  QueryResult result{
      PQexecParams(db_.get(), sql.text, static_cast<int>(parameters.size()),
                   nullptr, parameters.begin(), nullptr, nullptr, 0),
      PQclear};
  if (!result || PQresultStatus(result.get()) != expected)
    throw std::runtime_error{"database query failed"};
  return result;
}
