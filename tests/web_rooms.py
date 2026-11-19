"""Play from six independent authenticated clients in a disposable stack."""
import secrets
import sys
import time

from web_client import Client

base = sys.argv[1]
clients = [Client(base) for _ in range(7)]
names = []
for client in clients:
    name = "room-" + secrets.token_hex(4)
    names.append(name)
    client.call("auth/register", {"login": name, "password": secrets.token_urlsafe(24)}, 201)

guest = Client(base)
guest.call("rooms", {}, 401)
room = clients[0].call("rooms", {}, 201)["room"]
code = room["code"]

def action(client, name, view, expected=200, **params):
    return client.call("rooms/" + name, {"code": code, "version": view["version"], **params}, expected)


action(clients[0], "start", room, 409)
for index in range(1, 6):
    room = clients[index].call("rooms/join", {"code": code})["room"]
    assert room["seat"] == index and len(room["players"]) == index + 1
clients[6].call("rooms/join", {"code": code}, 409)
action(clients[6], "start", room, 404)
action(clients[1], "start", room, 403)
room = action(clients[0], "start", room)["room"]
clients[6].call("rooms/join", {"code": code}, 409)
hands = set()
for index, client in enumerate(clients[:6]):
    view = client.call("rooms/current")["room"]
    assert view["game"]["name"] == names[index] and view["seat"] == index
    assert len(view["game"]["hand"]) == 6
    assert not hands.intersection(view["game"]["hand"])
    hands.update(view["game"]["hand"])
    assert all(set(player) == {"name", "left", "cards"} for player in view["players"])
    assert bool(view["game"]["moves"]) == (view["turn"] == index)
assert len(hands) == 36
actor = room["turn"]
action(clients[(actor + 1) % 6], "moves", room, 409, move=0)
action(clients[actor], "moves", dict(room, version=room["version"]-1), 409, move=0)
# Refresh an authenticated seat with a new HTTP client.
refreshed = Client(base)
refreshed.cookies.update(clients[actor].cookies)
assert refreshed.call("rooms/current")["room"] == clients[actor].call("rooms/current")["room"]
clients[actor] = refreshed
action(clients[5], "leave", room)
assert clients[5].call("rooms/current")["room"] is None
room = clients[0].call("rooms/current")["room"]
assert room["players"][5]["left"]
turns = 0
while room["turn"] is not None:
    time.sleep(.24)
    actor = room["turn"]
    assert actor != 5
    mine = clients[actor].call("rooms/current")["room"]
    move = mine["game"]["moves"][0]
    room = action(clients[actor], "moves", mine, move=move)["room"]
    turns += 1
    assert turns < 2000
for client in clients[:5]:
    time.sleep(.15)
    finished = client.call("rooms/current")["room"]
    assert finished["game"]["result"] in ("win", "loss", "draw")
    action(client, "leave", finished)
assert clients[0].call("rooms/current")["room"] is None
# In a waiting room, leaving the first seat transfers hosting to the next member.
room = clients[0].call("rooms", {}, 201)["room"]
code = room["code"]
room = clients[1].call("rooms/join", {"code": code})["room"]
action(clients[0], "leave", room)
assert clients[1].call("rooms/current")["room"]["seat"] == 0
print("Six seats, private hands, turn ownership, reconnect, leave and game completion passed:", turns, "moves")
