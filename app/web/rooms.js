"use strict";

let onlineRoom = null;
let roomEpoch = 0;
let pollingRoom = false;
let reconnectingRoom = false;

function inviteCode(value) {
  const text = value.trim();
  if (/^[0-9a-f]{16}$/i.test(text)) return text.toLowerCase();
  try {
    const link = new URL(text);
    const code = new URLSearchParams(link.hash.slice(1)).get("room") ?? "";
    if (link.origin === location.origin && /^[0-9a-f]{16}$/i.test(code)) return code.toLowerCase();
  } catch { return ""; }
  return "";
}

function roomNote(text) {
  reconnectingRoom = false;
  byId("room-note").textContent = text;
  byId("room-note").hidden = !text;
}

function roomTurnText() {
  if (game.phase === "finished") {
    return {win: "You win! Your hand is empty.", loss: "You lose. You’re the last player holding cards.",
      draw: "Draw. The final players went out together."}[game.result];
  }
  if (onlineRoom.turn !== onlineRoom.seat) return "Waiting for " + onlineRoom.players[onlineRoom.turn].name + "…";
  if (game.phase === "pickup") return "The defender is picking up. Add a matching rank or pass.";
  return turnText[game.phase];
}

function renderRoom() {
  const room = onlineRoom;
  byId("room-panel").hidden = !room && Boolean(game) && !inviteCode(location.href);
  byId("room-entry").hidden = Boolean(room);
  byId("room-inside").hidden = !room;
  byId("room-join").hidden = !accountLogin;
  byId("room-signin").hidden = Boolean(accountLogin);
  byId("room-title").textContent = !room ? "Play with friends" :
    (room.game ? "Players" : "Waiting room · " + room.players.length + "/6");
  for (const button of byId("room-panel").querySelectorAll("button")) button.disabled = waiting;
  if (!room) return;
  byId("room-players").replaceChildren(...room.players.map((player, index) => {
    const details = [];
    if (index === room.seat) details.push("you");
    if (!room.game && index === 0) details.push("host");
    if (player.left) details.push("bot");
    if (room.game) details.push(countCards(player.cards));
    return element("li", room.turn === index ? "active" : "", player.name + (details.length ? " · " + details.join(", ") : ""));
  }));
  byId("room-invite-controls").hidden = Boolean(room.game);
  byId("room-link").value = location.origin + "/#room=" + room.code;
  byId("room-start").hidden = Boolean(room.game) || room.seat !== 0;
  byId("room-start").disabled = waiting || room.players.length < 2;
}

function adoptRoom(next) {
  const wasInRoom = Boolean(onlineRoom);
  onlineRoom = next;
  if (next || wasInRoom) {
    game = next?.game ?? null;
    needsSync = false;
  }
  render();
}

async function pollRoom() {
  if (!accountLogin || pollingRoom || waiting) return;
  const epoch = roomEpoch;
  pollingRoom = true;
  try {
    const {room} = await request("rooms/current");
    if (epoch !== roomEpoch || waiting) return;
    if (room?.code !== onlineRoom?.code || room?.version !== onlineRoom?.version) {
      adoptRoom(room);
      roomNote("");
    } else if (reconnectingRoom) roomNote("");
  } catch (error) {
    if (error.status === 401) location.reload();
    else if (onlineRoom) {
      roomNote("Connection lost. Reconnecting…");
      reconnectingRoom = true;
    }
  } finally { pollingRoom = false; }
}

async function roomAction(path, values = {}) {
  if (waiting) return;
  ++roomEpoch;
  waiting = true;
  render();
  roomNote("");
  const data = onlineRoom ? {code: onlineRoom.code, version: onlineRoom.version, ...values} : values;
  try {
    const {room} = await request(path, data);
    adoptRoom(room);
    if (path === "rooms/join" || path === "rooms/leave") history.replaceState(null, "", location.pathname);
  } catch (error) {
    if (error.status === 401) { location.reload(); return; }
    try {
      adoptRoom((await request("rooms/current")).room);
    } catch { /* The next poll retries the connection. */ }
    const messages = {400: "Check the room code or move.", 403: "Only the host can start the game.",
      404: "Room not found. Check the invitation.", 409: "The table changed, or the room is full or already playing.",
      422: "You can’t play that move.", 429: "Please wait a moment before trying again."};
    roomNote(messages[error.status] ?? "Couldn’t reach the room. Please try again.");
  } finally { waiting = false; render(); }
}

function setupRooms() {
  const prefill = () => {
    const code = inviteCode(location.href);
    if (code) byId("room-code").value = code;
  };
  prefill();
  window.addEventListener("hashchange", () => { prefill(); render(); });
  byId("room-create").addEventListener("click", () => roomAction("rooms"));
  byId("room-signin").addEventListener("click", () => byId("account-open").click());
  byId("room-join").addEventListener("submit", event => {
    event.preventDefault();
    const code = inviteCode(byId("room-code").value);
    if (!code) { roomNote("Enter the 16-character code or paste the invite link."); return; }
    roomAction("rooms/join", {code});
  });
  byId("room-start").addEventListener("click", () => roomAction("rooms/start"));
  byId("room-leave").addEventListener("click", () => {
    if (game && !game.result && !window.confirm("Leave this game? A bot will take your seat.")) return;
    roomAction("rooms/leave");
  });
  byId("room-copy").addEventListener("click", async () => {
    try { await navigator.clipboard.writeText(byId("room-link").value); roomNote("Link copied."); }
    catch { byId("room-link").select(); roomNote("Copy the selected invite link."); }
  });
  const refresh = () => { if (!document.hidden && (onlineRoom || !game)) pollRoom(); };
  window.setInterval(refresh, 2000);
  document.addEventListener("visibilitychange", refresh);
}
