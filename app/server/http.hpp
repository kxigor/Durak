#pragma once

#include <httplib.h>

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

using Json = nlohmann::json;
std::string new_id();
std::string cookie(const httplib::Request& request,
                   std::string_view prefix = "__Host-durak=");
void reply(httplib::Response& response, const Json& data);
std::optional<Json> payload(const httplib::Request& request,
                            httplib::Response& response);
bool in_range(const Json& value, int low, int high);
