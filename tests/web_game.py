"""Exercise a full browser game through the frontend proxy."""
import http.cookies
import json
import sys
import time
import urllib.error
import urllib.request

base = sys.argv[1] if len(sys.argv) > 1 else "http://localhost:8189"
cookie = ""


def call(path, data=None, expected=200, authenticated=True):
    global cookie
    headers = {"Cookie": cookie} if authenticated else {}
    if data is not None:
        headers["Content-Type"] = "application/json"
    body = json.dumps(data).encode() if data is not None else None
    request = urllib.request.Request(base + path, body, headers)
    try:
        response = urllib.request.urlopen(request, timeout=5)
    except urllib.error.HTTPError as error:
        response = error
    with response:
        assert response.status == expected, (path, response.status, expected)
        if value := response.headers.get("Set-Cookie"):
            parsed = http.cookies.SimpleCookie(value)
            morsel = parsed["__Host-durak"]
            assert morsel["secure"] and morsel["httponly"] and morsel["samesite"] == "Strict"
            cookie = "__Host-durak=" + morsel.value
        data = response.read()
        if response.headers.get_content_type() == "application/json":
            return json.loads(data)
        return data


assert b"<html" in call("/")
assert b"font-size" in call("/style.css")
assert call("/api/health") == {"ok": True}
call("/api/game", expected=404)
name = "smoke-" + str(time.time_ns())[-12:]
state = call("/api/games", {"name": name}, 201)
call("/api/game", expected=404, authenticated=False)
assert state == call("/api/game")
call("/api/moves", {"move": 99, "version": 0}, 400)
call("/api/moves", {"move": state["moves"][0], "version": 1}, 409)
for turn in range(1500):
    assert "bot_hand" not in state and state["moves"]
    time.sleep(0.11)
    state = call("/api/moves", {"move": state["moves"][0], "version": state["version"]})
    if state["result"]:
        break
assert state["phase"] == "finished"
row = next(row for row in call("/api/leaderboard")["leaders"] if row["name"] == name)
assert row[{"win": "wins", "loss": "losses", "draw": "draws"}[state["result"]]] == 1
assert sum(row[k] for k in ("wins", "losses", "draws")) == 1
print("Full game passed:", state["version"], "moves,", state["result"])
