#pragma once

#include <libpq-fe.h>

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class Scores {
 public:
  Scores();
  void record(const std::string& game, const std::string& name,
              const std::string& outcome);
  nlohmann::json leaders();

 private:
  PGconn* connection();
  std::unique_ptr<PGconn, decltype(&PQfinish)> db_;
};
