#pragma once

#include <httplib.h>

#include <chrono>
#include <map>
#include <mutex>
#include <string>

#include "database.hpp"

class Accounts {
 public:
  Accounts();
  void routes(httplib::Server& server);
  std::string current(const httplib::Request& request);

 private:
  using Clock = std::chrono::steady_clock;
  struct Attempts {
    int failures{};
    Clock::time_point until;
  };

  void authenticate(const httplib::Request& req, httplib::Response& res,
                    bool create);
  void logout(const httplib::Request& req, httplib::Response& res);

  Database db_;
  std::mutex database_mutex_;
  std::mutex password_mutex_;
  std::map<std::string, Attempts> attempts_;
  std::string dummy_hash_;
};
