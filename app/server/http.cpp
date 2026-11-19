#include "http.hpp"

#include <openssl/rand.h>

#include <array>
#include <stdexcept>

std::string new_id() {
  std::array<unsigned char, 32> bytes;
  if (RAND_bytes(bytes.data(), static_cast<int>(bytes.size())) != 1)
    throw std::runtime_error{"random source unavailable"};
  constexpr std::string_view kHex{"0123456789abcdef"};
  std::string id;
  for (const auto byte : bytes) {
    id.push_back(kHex[byte >> 4]);
    id.push_back(kHex[byte & 15U]);
  }
  return id;
}

std::string cookie(const httplib::Request& request, std::string_view prefix) {
  const auto cookies = request.get_header_value("Cookie");
  std::string_view remaining{cookies};
  while (!remaining.empty()) {
    const auto separator = remaining.find(';');
    auto item = remaining.substr(0, separator);
    while (item.starts_with(' ')) item.remove_prefix(1);
    if (item.starts_with(prefix) && item.size() == prefix.size() + 64)
      return std::string{item.substr(prefix.size())};
    if (separator == std::string_view::npos) break;
    remaining.remove_prefix(separator + 1);
  }
  return {};
}

void reply(httplib::Response& response, const Json& data) {
  response.set_content(data.dump(), "application/json; charset=utf-8");
}

std::optional<Json> payload(const httplib::Request& request,
                            httplib::Response& response) {
  const auto site = request.get_header_value("Sec-Fetch-Site");
  if (request.get_header_value("Content-Type") != "application/json" ||
      (!site.empty() && site != "same-origin" && site != "none")) {
    response.status = 403;
    return {};
  }
  auto data = Json::parse(request.body, nullptr, false);
  if (!data.is_object()) {
    response.status = 400;
    return {};
  }
  return data;
}

bool in_range(const Json& value, int low, int high) {
  if (value.is_number_unsigned()) {
    return value.get<Json::number_unsigned_t>() <= static_cast<unsigned>(high);
  }
  if (!value.is_number_integer()) return false;
  const auto number = value.get<Json::number_integer_t>();
  return number >= low && number <= high;
}
