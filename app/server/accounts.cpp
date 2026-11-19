#include "accounts.hpp"

#include <sodium.h>

#include <algorithm>
#include <array>
#include <stdexcept>

#include "http.hpp"

namespace {
constexpr std::string_view kCookie{"__Host-durak-account="};
constexpr std::string_view kFlags{
    "; Path=/; Secure; HttpOnly; SameSite=Strict; Max-Age="};

std::string hash_password(const std::string& password) {
  std::array<char, crypto_pwhash_STRBYTES> hash{};
  if (crypto_pwhash_str_alg(hash.data(), password.data(), password.size(), 3,
                            32U * 1024U * 1024U,
                            crypto_pwhash_ALG_ARGON2ID13) != 0)
    throw std::runtime_error{"password hashing unavailable"};
  return hash.data();
}

std::string token_digest(const std::string& token) {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  crypto_hash_sha256(digest.data(),
                     reinterpret_cast<const unsigned char*>(token.data()),
                     token.size());
  std::array<char, 2 * crypto_hash_sha256_BYTES + 1> hex{};
  sodium_bin2hex(hex.data(), hex.size(), digest.data(), digest.size());
  return hex.data();
}

bool canonicalize(std::string& login) {
  if (login.size() < 3 || login.size() > 20) return false;
  for (char& ch : login) {
    if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch - 'A' + 'a');
    if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' ||
          ch == '-'))
      return false;
  }
  return true;
}
}  // namespace

Accounts::Accounts() {
  if (sodium_init() < 0)
    throw std::runtime_error{"password hashing unavailable"};
  dummy_hash_ = hash_password(new_id());
}

std::string Accounts::current(const httplib::Request& req) {
  const auto token = cookie(req, kCookie);
  if (token.empty()) return {};
  const auto digest = token_digest(token);
  std::scoped_lock lock{database_mutex_};
  auto rows = db_.query(
      "SELECT login FROM accounts WHERE session_digest=$1 AND expires_at>now()",
      {digest.c_str()});
  return PQntuples(rows.get()) == 1 ? PQgetvalue(rows.get(), 0, 0) : "";
}

void Accounts::routes(httplib::Server& server) {
  server.Get("/api/auth/me", [this](const auto& req, auto& res) {
    reply(res, {{"login", current(req)}});
  });
  server.Post("/api/auth/register", [this](const auto& req, auto& res) {
    authenticate(req, res, true);
  });
  server.Post("/api/auth/login", [this](const auto& req, auto& res) {
    authenticate(req, res, false);
  });
  server.Post("/api/auth/logout",
              [this](const auto& req, auto& res) { logout(req, res); });
}

void Accounts::authenticate(const httplib::Request& req, httplib::Response& res,
                            bool create) {
  auto data = payload(req, res);
  if (!data) return;
  if (!data->contains("login") || !data->at("login").is_string() ||
      !data->contains("password") || !data->at("password").is_string()) {
    res.status = 400;
    return;
  }
  auto login = data->at("login").get<std::string>();
  const auto& password = data->at("password").get_ref<const std::string&>();
  const auto characters = std::ranges::count_if(
      password, [](unsigned char byte) { return (byte & 0xc0U) != 0x80U; });
  if (!canonicalize(login) || password.empty() || password.size() > 128 ||
      (create && characters < 15)) {
    res.status = 400;
    return;
  }
  // Keep concurrent Argon2 work within the container's memory budget.
  std::unique_lock hash_lock{password_mutex_, std::try_to_lock};
  if (!hash_lock.owns_lock()) {
    res.status = 429;
    return;
  }
  const auto now = Clock::now();
  std::erase_if(attempts_,
                [&](const auto& item) { return item.second.until <= now; });
  const auto prior = attempts_.find(login);
  if (!create && prior != attempts_.end() && prior->second.failures >= 8) {
    res.status = 429;
    return;
  }
  const auto token = new_id();
  const auto digest = token_digest(token);
  if (create) {
    const auto hash = hash_password(password);
    std::scoped_lock lock{database_mutex_};
    auto rows = db_.query(
        "INSERT INTO accounts (login,password_hash,session_digest,expires_at) "
        "VALUES ($1,$2,$3,now()+interval '7 days') ON CONFLICT (login) DO "
        "NOTHING RETURNING login",
        {login.c_str(), hash.c_str(), digest.c_str()});
    if (!PQntuples(rows.get())) {
      res.status = 409;
      return;
    }
    res.status = 201;
  } else {
    std::string stored;
    {
      std::scoped_lock lock{database_mutex_};
      auto rows = db_.query("SELECT password_hash FROM accounts WHERE login=$1",
                            {login.c_str()});
      if (PQntuples(rows.get()) == 1) stored = PQgetvalue(rows.get(), 0, 0);
    }
    if (crypto_pwhash_str_verify(
            stored.empty() ? dummy_hash_.c_str() : stored.c_str(),
            password.data(), password.size()) != 0 ||
        stored.empty()) {
      if (!stored.empty()) {
        auto [it, inserted] = attempts_.try_emplace(
            login, Attempts{0, now + std::chrono::minutes{10}});
        ++it->second.failures;
      }
      res.status = 401;
      return;
    }
    std::scoped_lock lock{database_mutex_};
    db_.query(
        "UPDATE accounts SET session_digest=$2,expires_at=now()+interval '7 "
        "days' WHERE login=$1",
        {login.c_str(), digest.c_str()}, PGRES_COMMAND_OK);
    attempts_.erase(login);
  }
  res.set_header("Set-Cookie",
                 std::string{kCookie} + token + std::string{kFlags} + "604800");
  reply(res, {{"login", login}});
}

void Accounts::logout(const httplib::Request& req, httplib::Response& res) {
  if (!payload(req, res)) return;
  const auto digest = token_digest(cookie(req, kCookie));
  std::scoped_lock lock{database_mutex_};
  db_.query(
      "UPDATE accounts SET session_digest=NULL,expires_at=NULL WHERE "
      "session_digest=$1",
      {digest.c_str()}, PGRES_COMMAND_OK);
  res.set_header("Set-Cookie",
                 std::string{kCookie} + std::string{kFlags} + "0");
  reply(res, {{"login", ""}});
}
