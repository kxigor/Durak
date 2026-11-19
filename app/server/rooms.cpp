#include "rooms.hpp"

#include <algorithm>
#include <random>

using durak::rules::Match;
using durak::rules::Phase;

std::optional<std::size_t> Rooms::Room::member(const std::string& login) const {
  const auto found = std::ranges::find_if(seats, [&](const Seat& seat) {
    return !seat.left && seat.login == login;
  });
  if (found == seats.end()) return std::nullopt;
  return static_cast<std::size_t>(found - seats.begin());
}

void Rooms::Room::advance_bots() {
  // Bound work under the room lock if only departed seats remain in the round.
  for (unsigned step = 0;
       step < 64 && match && match->phase() != Phase::Finished &&
       seats.at(match->actor()).left;
       ++step) {
    match->play(match->computer_move());
    ++version;
  }
}

Json Rooms::Room::view(const std::string& code, std::size_t seat) const {
  auto roster = Json::array();
  for (std::size_t index = 0; index < seats.size(); ++index) {
    Json player = {{"name", seats[index].login}, {"left", seats[index].left}};
    if (match)
      player["cards"] = match->board().players()[index]->get_card_count();
    roster.push_back(std::move(player));
  }
  Json room = {
      {"code", code},       {"seat", seat},    {"players", std::move(roster)},
      {"version", version}, {"game", nullptr}, {"turn", nullptr}};
  if (match) {
    room["game"] = match_state(*match, seat, seats.at(seat).login, version);
    if (match->phase() != Phase::Finished) room["turn"] = match->actor();
  }
  return room;
}

void Rooms::routes(httplib::Server& server) {
  server.Get("/api/rooms/current", [this](const auto& req, auto& res) {
    handle(Action::Current, req, res);
  });
  const auto post = [&](const char* path, Action action) {
    server.Post(path, [this, action](const auto& req, auto& res) {
      handle(action, req, res);
    });
  };
  post("/api/rooms", Action::Create);
  post("/api/rooms/join", Action::Join);
  post("/api/rooms/start", Action::Start);
  post("/api/rooms/moves", Action::Move);
  post("/api/rooms/leave", Action::Leave);
}

void Rooms::handle(Action action, const httplib::Request& req,
                   httplib::Response& res) {
  const auto login = accounts_.current(req);
  if (login.empty()) {
    res.status = 401;
    return;
  }
  Json data;
  if (action != Action::Current) {
    auto parsed = payload(req, res);
    if (!parsed) return;
    data = std::move(*parsed);
  }
  std::scoped_lock lock{mutex_};
  const auto now = Clock::now();
  std::erase_if(rooms_, [&](const auto& item) {
    return now - item.second.updated >= std::chrono::hours{2};
  });
  auto own = std::ranges::find_if(rooms_, [&](const auto& item) {
    return item.second.member(login).has_value();
  });
  if (action == Action::Create && own == rooms_.end()) {
    if (rooms_.size() >= 256) {
      res.status = 503;
      return;
    }
    auto [room, added] = rooms_.try_emplace(new_id().substr(0, 16));
    if (!added) {
      res.status = 503;
      return;
    }
    room->second.seats.push_back(Seat{login});
    own = room;
    res.status = 201;
  }
  if (action != Action::Create && action != Action::Current) {
    if (!data.contains("code") || !data.at("code").is_string()) {
      res.status = 400;
      return;
    }
    auto requested = rooms_.find(data.at("code").get_ref<const std::string&>());
    if (requested == rooms_.end()) {
      res.status = 404;
      return;
    }
    if (action == Action::Join) {
      if (own != requested) {
        auto& room = requested->second;
        if (own != rooms_.end() || room.match ||
            room.seats.size() >= Match::kMaxPlayers) {
          res.status = 409;
          return;
        }
        room.seats.push_back(Seat{login});
        ++room.version;
        room.updated = now;
        own = requested;
      }
    } else {
      if (own != requested) {
        res.status = 404;
        return;
      }
      auto& room = own->second;
      if (!data.contains("version") ||
          !in_range(data.at("version"), 0, 1000000000)) {
        res.status = 400;
        return;
      }
      if (data.at("version") != room.version) {
        res.status = 409;
        return;
      }
      act(action, room, *room.member(login), data, res);
      if (res.status >= 400) return;
      ++room.version;
      room.updated = now;
      if (action == Action::Leave) {
        if (std::ranges::none_of(room.seats,
                                 [](const Seat& p) { return !p.left; }))
          rooms_.erase(own);
        reply(res, {{"room", nullptr}});
        return;
      }
    }
  }
  if (own == rooms_.end()) {
    reply(res, {{"room", nullptr}});
    return;
  }
  own->second.advance_bots();
  reply(res,
        {{"room", own->second.view(own->first, *own->second.member(login))}});
}

void Rooms::act(Action action, Room& room, std::size_t seat, const Json& data,
                httplib::Response& res) {
  if (action == Action::Start) {
    if (seat != 0) {
      res.status = 403;
      return;
    }
    if (room.match || room.seats.size() < 2) {
      res.status = 409;
      return;
    }
    room.match.emplace(room.seats.size(), std::random_device{}());
  } else if (action == Action::Leave) {
    if (room.match)
      room.seats.at(seat).left = true;
    else
      room.seats.erase(room.seats.begin() + static_cast<std::ptrdiff_t>(seat));
  } else {
    if (!room.match || room.match->phase() == Phase::Finished ||
        room.match->actor() != seat) {
      res.status = 409;
      return;
    }
    if (!data.contains("move") || !in_range(data.at("move"), -2, 51)) {
      res.status = 400;
      return;
    }
    try {
      room.match->play(data.at("move").get<int>());
    } catch (const std::invalid_argument&) {
      res.status = 422;
    }
  }
}
