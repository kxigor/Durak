#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "accounts.hpp"
#include "http.hpp"
#include "match_state.hpp"

class Rooms {
 public:
  explicit Rooms(Accounts& accounts) : accounts_{accounts} {}
  void routes(httplib::Server& server);

 private:
  using Clock = std::chrono::steady_clock;
  enum class Action { Current, Create, Join, Start, Move, Leave };
  struct Seat {
    std::string login;
    bool left{};
  };
  struct Room {
    std::optional<std::size_t> member(const std::string& login) const;
    void advance_bots();
    Json view(const std::string& code, std::size_t seat) const;

    std::vector<Seat> seats;
    std::optional<durak::rules::Match> match;
    int version{};
    Clock::time_point updated{Clock::now()};
  };

  void handle(Action action, const httplib::Request& req,
              httplib::Response& res);
  void act(Action action, Room& room, std::size_t seat, const Json& data,
           httplib::Response& res);

  Accounts& accounts_;
  std::mutex mutex_;
  std::map<std::string, Room> rooms_;
};
