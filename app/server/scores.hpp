#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "database.hpp"

class Scores {
 public:
  void record(const std::string& game, const std::string& name,
              const std::string& outcome, const std::string& owner);
  nlohmann::json leaders();

 private:
  Database db_;
};
