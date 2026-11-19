"""Run against a disposable Compose project."""
import secrets
import sys
import time

from web_client import Client

base = sys.argv[1]
first, second = Client(base), Client(base)
name = "member-" + secrets.token_hex(4)
credentials = {"login": name, "password": secrets.token_urlsafe(24)}
assert first.call("auth/me") == {"login": ""}
first.call("auth/register", dict(credentials, password="short"), 400)
first.call("auth/register", credentials, 403, {"Sec-Fetch-Site": "cross-site"})
assert first.call("auth/register", credentials, 201)["login"] == name
second.call("auth/register", dict(credentials, login=name.upper()), 409)
assert first.call("auth/me")["login"] == name
second.call("auth/login", dict(credentials, password=secrets.token_urlsafe(20)), 401)
game = first.call("games", {"name": "someone-else"}, 201)
assert game["name"] == name
while not game["result"]:
    assert game["version"] < 1500
    time.sleep(0.12)
    game = first.call("moves", {"move": game["moves"][0], "version": game["version"]})
assert first.call("game") == game
score = next(row for row in first.call("leaderboard")["leaders"] if row["name"] == name)
assert score["registered"] and score[{"win": "wins", "loss": "losses", "draw": "draws"}[game["result"]]] == 1
assert sum(score[field] for field in ("wins", "losses", "draws")) == 1
second.call("auth/login", dict(credentials, login=name.upper()))
assert first.call("auth/me")["login"] == ""
first.call("game", expected=404)
old_cookie = second.cookies.copy()
second.call("auth/logout", {})
assert second.call("auth/me")["login"] == ""
second.cookies.update(old_cookie)
assert second.call("auth/me")["login"] == ""
print("Account registration, protected scores, sign-in and sign-out passed")
