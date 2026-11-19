#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "database.hpp"

class Scores {
 public:
  void record(const std::string& game, const std::string& owner,
              const std::string& outcome);
  nlohmann::json leaders();

 private:
  Database db_;
};
