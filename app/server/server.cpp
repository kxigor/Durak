#include <httplib.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <include/durak/rules/match.hpp>
#include <iostream>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>

#include "accounts.hpp"
#include "http.hpp"
#include "scores.hpp"

namespace {
using durak::rules::Match;
using durak::rules::Phase;
using Clock = std::chrono::steady_clock;

struct Session {
  Session(std::string nickname, std::string account)
      : name{std::move(nickname)}, owner{std::move(account)} {}
  Match match;
  std::string score_id;
  std::string name;
  std::string owner;
  int version{};
  Clock::time_point seen{Clock::now()};
  bool recorded{};
};

std::string_view phase_name(Phase phase) {
  switch (phase) {
    case Phase::Attack:
      return "attack";
    case Phase::Defend:
      return "defend";
    case Phase::Throw:
      return "throw";
    case Phase::Pickup:
      return "pickup";
    case Phase::Finished:
      return "finished";
    default:
      throw std::logic_error{"unknown phase"};
  }
}

Json snapshot(const Session& session) {
  const auto& match = session.match;
  const auto& board = match.board();
  auto hand = Json::array();
  for (const auto card : board.players()[0]->get_cards())
    hand.push_back(card.id);
  auto table = Json::array();
  for (const auto& pair : board.table().get_cards()) {
    table.push_back(
        {{"attack", pair.get_attack_card().id},
         {"defense",
          pair.is_beaten() ? Json(pair.get_beat_card().id) : Json(nullptr)}});
  }
  auto result = match.result();
  return {{"name", session.name},
          {"hand", hand},
          {"table", table},
          {"bot_count", board.players()[1]->get_card_count()},
          {"stock_count", board.stock().size()},
          {"trump", board.trump().id},
          {"phase", phase_name(match.phase())},
          {"result", result.empty() ? Json(nullptr) : Json(result)},
          {"moves", match.legal_moves()},
          {"version", session.version}};
}

bool valid_name(const std::string& name) {
  if (name.empty() || name.size() > 80 || name.front() == ' ' ||
      name.back() == ' ')
    return false;
  std::size_t count{};
  for (const char character : name) {
    const auto byte = static_cast<unsigned char>(character);
    if (byte < 32 || byte == 127) return false;
    if ((byte & 0xc0U) != 0x80U) ++count;
  }
  return count <= 20;
}

bool in_range(const Json& value, int low, int high) {
  if (value.is_number_unsigned()) {
    return value.get<Json::number_unsigned_t>() <= static_cast<unsigned>(high);
  }
  if (!value.is_number_integer()) return false;
  const auto number = value.get<Json::number_integer_t>();
  return number >= low && number <= high;
}

class Games {
 public:
  void routes(httplib::Server& server) {
    accounts_.routes(server);
    server.Get("/api/health",
               [](const auto&, auto& res) { reply(res, {{"ok", true}}); });
    server.Get("/api/leaderboard", [this](const auto&, auto& res) {
      std::scoped_lock lock{mutex_};
      reply(res, {{"leaders", scores_.leaders()}});
    });
    server.Post("/api/games",
                [this](const auto& req, auto& res) { create(req, res); });
    server.Get("/api/game",
               [this](const auto& req, auto& res) { access(req, res, false); });
    server.Post("/api/moves",
                [this](const auto& req, auto& res) { access(req, res, true); });
  }

 private:
  void create(const httplib::Request& req, httplib::Response& res) {
    auto data = payload(req, res);
    if (!data) return;
    const auto owner = accounts_.current(req);
    std::string name = owner;
    if (owner.empty()) {
      if (!data->contains("name") || !data->at("name").is_string() ||
          !valid_name(data->at("name").get_ref<const std::string&>())) {
        res.status = 400;
        return;
      }
      name = data->at("name").get<std::string>();
    }
    std::scoped_lock lock{mutex_};
    const auto cutoff = Clock::now() - std::chrono::hours{2};
    std::erase_if(sessions_, [cutoff](const auto& entry) {
      return entry.second.seen < cutoff;
    });
    const auto previous = sessions_.find(cookie(req));
    if (previous != sessions_.end() && previous->second.owner == owner) {
      if (previous->second.match.phase() != Phase::Finished) {
        reply(res, snapshot(previous->second));
        return;
      }
      record(previous->second);
      sessions_.erase(previous);
    }
    if (sessions_.size() >= 512) {
      res.status = 503;
      return;
    }
    const auto id = new_id();
    auto entry = sessions_.try_emplace(id, name, owner).first;
    res.set_header(
        "Set-Cookie",
        "__Host-durak=" + id +
            "; Path=/; HttpOnly; Secure; SameSite=Strict; Max-Age=7200");
    res.status = 201;
    reply(res, snapshot(entry->second));
  }

  void record(Session& session) {
    if (!session.recorded && session.match.phase() == Phase::Finished) {
      if (session.score_id.empty()) session.score_id = new_id();
      scores_.record(session.score_id, session.name,
                     std::string{session.match.result()}, session.owner);
      session.recorded = true;
    }
  }

  void access(const httplib::Request& req, httplib::Response& res,
              bool update) {
    std::optional<Json> data;
    if (update) {
      data = payload(req, res);
      if (!data) return;
      if (!data->contains("move") || !data->contains("version") ||
          !in_range(data->at("move"), -2, 51) ||
          !in_range(data->at("version"), 0, 1000000000)) {
        res.status = 400;
        return;
      }
    }
    const auto owner = accounts_.current(req);
    std::scoped_lock lock{mutex_};
    auto it = sessions_.find(cookie(req));
    if (it == sessions_.end() || it->second.owner != owner ||
        it->second.seen + std::chrono::hours{2} < Clock::now()) {
      res.status = 404;
      return;
    }
    auto& session = it->second;
    session.seen = Clock::now();
    record(session);
    if (data) {
      if (data->at("version") != session.version) {
        res.status = 409;
        return;
      }
      try {
        session.match.play(data->at("move").get<int>());
      } catch (const std::invalid_argument&) {
        res.status = 422;
        return;
      }
      ++session.version;
      record(session);
    }
    reply(res, snapshot(session));
  }

  Accounts accounts_;
  Scores scores_;
  std::map<std::string, Session> sessions_;
  std::mutex mutex_;
};
}  // namespace

int main() {
  try {
    Games games;
    httplib::Server server;
    server.new_task_queue = [] { return new httplib::ThreadPool{4, 32}; };
    server.set_payload_max_length(512);
    server.set_read_timeout(5, 0);
    server.set_write_timeout(5, 0);
    server.set_keep_alive_max_count(32);
    server.set_exception_handler(
        [](const auto&, auto& res, std::exception_ptr) {
          res.status = 500;
          reply(res, {{"error", "internal error"}});
        });
    games.routes(server);
    return server.listen("0.0.0.0", 8080) ? 0 : 1;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
