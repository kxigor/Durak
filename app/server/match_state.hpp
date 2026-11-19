#pragma once

#include <include/durak/rules/match.hpp>
#include <nlohmann/json.hpp>
#include <string>

nlohmann::json match_state(const durak::rules::Match& match, std::size_t seat,
                           const std::string& name, int version);
