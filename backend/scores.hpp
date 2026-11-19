#pragma once

#include <sqlite3.h>

#include <nlohmann/json.hpp>
#include <string>

class Scores {
 public:
  explicit Scores(const char* path);
  Scores(const Scores&) = delete;
  Scores& operator=(const Scores&) = delete;
  ~Scores();
  void record(const std::string& game, const std::string& name,
              const std::string& outcome);
  nlohmann::json leaders() const;

 private:
  sqlite3* db_{};
};
