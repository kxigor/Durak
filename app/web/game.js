"use strict";

const byId = (id) => document.getElementById(id);
const lobby = byId("lobby");
const gameArea = byId("game");
const notice = byId("notice");
const nick = byId("player-name");
const syncButton = byId("sync");

const suits = ["♣", "♦", "♥", "♠"];
const suitNames = ["clubs", "diamonds", "hearts", "spades"];
const figures = {11: "J", 12: "Q", 13: "K", 14: "A"};
const faceNames = {11: "Jack", 12: "Queen", 13: "King", 14: "Ace"};
const turnText = {
  attack: "Your turn. Choose a card to attack.",
  defend: "Your turn to defend. Beat the card or pick up the pile.",
  throw: "Play a matching rank or pass.",
  pickup: "The bot is picking up. Add a matching rank or end the round."
};
const resultText = {win: "You win! You played your last card.", loss: "You lose. The bot played its last card.", draw: "Draw. Both hands are empty."};

let accountLogin = "";
let game = null;
let waiting = false;
let needsSync = false;
let scoreRequest = 0;

function element(tag, className, text) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  if (text !== undefined) node.textContent = text;
  return node;
}

function cardName(id) {
  const rank = Math.floor(id / 4) + 2;
  return (faceNames[rank] ?? rank) + " of " + suitNames[id % 4];
}

function card(id, inHand) {
  const suit = id % 4;
  const rank = Math.floor(id / 4) + 2;
  const node = element(inHand ? "button" : "div", "card");
  node.classList.toggle("red", suit === 1 || suit === 2);
  if (inHand) {
    node.type = "button";
    node.disabled = waiting || needsSync || !game.moves.includes(id);
    node.addEventListener("click", () => play(id));
  } else {
    node.setAttribute("role", "img");
  }
  node.setAttribute("aria-label", cardName(id));
  node.title = cardName(id);
  const corner = (figures[rank] ?? rank) + suits[suit];
  node.append(element("span", "card-rank", corner), element("span", "card-suit", suits[suit]), element("span", "card-bottom", corner));
  return node;
}

function countCards(count) {
  return count + (count === 1 ? " card" : " cards");
}

function message(text) {
  notice.textContent = text;
  notice.hidden = !text;
}

function render() {
  renderRoom();
  lobby.hidden = Boolean(game) || needsSync || Boolean(onlineRoom);
  gameArea.hidden = !game;
  byId("loading").hidden = true;
  syncButton.hidden = !needsSync;
  syncButton.disabled = waiting;
  byId("join").querySelector("button").disabled = waiting;
  byId("join").setAttribute("aria-busy", String(waiting));
  if (!game) return;

  const trump = byId("trump");
  const trumpSuit = game.trump % 4;
  const trumpRank = Math.floor(game.trump / 4) + 2;
  trump.textContent = (figures[trumpRank] ?? trumpRank) + " " + suits[trumpSuit];
  trump.classList.toggle("red", trumpSuit === 1 || trumpSuit === 2);
  trump.setAttribute("aria-label", cardName(game.trump));
  byId("stock-count").textContent = countCards(game.stock_count);
  byId("opponent-title").textContent = onlineRoom ? "Online table" : "Bot";
  byId("opponent-summary").hidden = Boolean(onlineRoom);
  if (!onlineRoom) byId("opponent-count").textContent = countCards(game.bot_count);

  const table = byId("table");
  table.replaceChildren();
  if (game.table.length === 0) table.append(element("p", "empty", "No cards on the table"));
  for (const pair of game.table) {
    const cards = element("div", "table-pair");
    const attack = card(pair.attack, false);
    attack.setAttribute("aria-label", "Attacking card: " + cardName(pair.attack));
    cards.append(attack);
    if (pair.defense !== null) {
      const defense = card(pair.defense, false);
      defense.setAttribute("aria-label", "Defending card: " + cardName(pair.defense));
      cards.append(defense);
    }
    table.append(cards);
  }

  gameArea.setAttribute("aria-busy", String(waiting));
  const finished = game.phase === "finished";
  byId("turn").textContent = onlineRoom ? roomTurnText() :
    (finished ? resultText[game.result] : turnText[game.phase]);
  byId("hand-count").textContent = countCards(game.hand.length);
  byId("hand").replaceChildren(...game.hand.map((id) => card(id, true)));
  byId("take").hidden = !game.moves.includes(-1);
  byId("done").hidden = !game.moves.includes(-2);
  byId("done").textContent = !onlineRoom && game.phase === "pickup" ? "End round" : "Pass";
  byId("again").hidden = !finished || Boolean(onlineRoom);
  for (const control of byId("actions").querySelectorAll("button")) control.disabled = waiting || needsSync;
}

async function request(path, body) {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 12000);
  try {
    const options = {signal: controller.signal, credentials: "same-origin", cache: "no-store"};
    if (body !== undefined) {
      options.method = "POST";
      options.headers = {"Content-Type": "application/json"};
      options.body = JSON.stringify(body);
    }
    const response = await fetch("/api/" + path, options);
    if (!response.ok) {
      const error = new Error("HTTP " + response.status);
      error.status = response.status;
      throw error;
    }
    return await response.json();
  } finally {
    clearTimeout(timeout);
  }
}

async function loadGame() {
  try {
    game = await request("game");
    nick.value = game.name;
  } catch (error) {
    if (error.status !== 404) throw error;
    game = null;
  }
  needsSync = false;
}

async function synchronize() {
  if (waiting) return;
  waiting = true;
  syncButton.disabled = true;
  message("");
  try {
    await loadGame();
    if (game?.phase === "finished") refreshScores();
  } catch {
    needsSync = true;
    message("Couldn’t load the game. Check your connection and try again.");
  } finally {
    waiting = false;
    render();
  }
}

async function submit(path, payload) {
  if (waiting || needsSync) return;
  const keyboardInput = document.activeElement?.matches(":focus-visible");
  waiting = true;
  message("");
  render();
  try {
    game = await request(path, payload);
    nick.value = game.name;
    if (game.phase === "finished") refreshScores();
  } catch (error) {
    if (error.status === 400 || error.status === 422) {
      message(path === "games" ? "Choose a different nickname and try again." : "You can’t play that move.");
    } else if (error.status === 429) {
      message("Too many requests. Wait a moment and try again.");
    } else {
      try {
        await loadGame();
        message(game ? "Game refreshed." : "You have no active game. Start a new one to play.");
        if (game?.phase === "finished") refreshScores();
      } catch {
        needsSync = true;
        message("Connection lost. Refresh the game before your next move.");
      }
    }
  } finally {
    waiting = false;
    render();
    if (keyboardInput) {
      const next = needsSync ? syncButton : game
        ? gameArea.querySelector("button:enabled:not([hidden])")
        : nick;
      next?.focus({preventScroll: true});
    }
  }
}

function play(move) {
  if (!game?.moves.includes(move)) return;
  if (onlineRoom) roomAction("rooms/moves", {move});
  else submit("moves", {move, version: game.version});
}

async function refreshScores() {
  const requestId = ++scoreRequest;
  byId("score-retry").hidden = true;
  try {
    const {leaders} = await request("leaderboard");
    if (requestId !== scoreRequest) return;
    const rows = leaders.map((entry) => {
      const row = element("tr");
      for (const field of ["name", "wins", "losses", "draws"]) row.append(element("td", "", entry[field]));
      if (entry.registered) {
        const badge = element("span", "member", "✓");
        badge.title = "Registered player";
        badge.setAttribute("aria-label", badge.title);
        row.firstChild.append(badge);
      }
      return row;
    });
    byId("leaders").replaceChildren(...rows);
    byId("score-note").textContent = rows.length ? "" : "Finish a game to appear on the leaderboard.";
    byId("score-note").hidden = Boolean(rows.length);
  } catch {
    if (requestId !== scoreRequest) return;
    byId("score-note").hidden = false;
    byId("score-note").textContent = "Couldn’t refresh the leaderboard.";
    byId("score-retry").hidden = false;
  }
}

byId("rules-open").addEventListener("click", () => byId("rules").showModal());
syncButton.addEventListener("click", synchronize);
byId("score-retry").addEventListener("click", refreshScores);
byId("take").addEventListener("click", () => play(-1));
byId("done").addEventListener("click", () => play(-2));
byId("again").addEventListener("click", () => {
  game = null;
  message("");
  render();
  nick.focus();
});
nick.addEventListener("invalid", () => {
  if (nick.validity.valueMissing) nick.setCustomValidity("Enter a nickname.");
});
nick.addEventListener("input", () => nick.setCustomValidity(""));
byId("join").addEventListener("submit", (event) => {
  event.preventDefault();
  const name = nick.value.trim();
  if (!name) {
    nick.setCustomValidity("Enter a nickname.");
    nick.reportValidity();
    return;
  }
  submit("games", {name});
});
let signingUp = false;
function accountMode(register) {
  signingUp = register;
  byId("account-title").textContent = register ? "Create account" : "Sign in";
  byId("account-submit").textContent = byId("account-title").textContent;
  byId("account-switch").textContent = register ? "Sign in instead" : "Create account";
  byId("account-help").hidden = !register;
  byId("password").autocomplete = register ? "new-password" : "current-password";
  byId("password").minLength = register ? 15 : 1;
  byId("account-note").hidden = true;
}
byId("account-open").addEventListener("click", () => {
  accountMode(false);
  byId("account").showModal();
});
byId("account-switch").addEventListener("click", () => accountMode(!signingUp));
byId("account-close").addEventListener("click", () => byId("account").close());
byId("account").addEventListener("close", () => { byId("password").value = ""; });
byId("account-form").addEventListener("submit", async event => {
  event.preventDefault();
  const submit = byId("account-submit");
  if (submit.disabled) return;
  submit.disabled = true;
  try {
    await request("auth/" + (signingUp ? "register" : "login"), {
      login: byId("login").value, password: byId("password").value
    });
    location.reload();
  } catch (error) {
    const errors = {400: "Check your username and password length.", 401: "Incorrect username or password.",
      409: "This username is taken.", 429: "Too many attempts. Please wait and try again."};
    byId("account-note").textContent = errors[error.status] ?? "Couldn’t reach your account. Try again.";
    byId("account-note").hidden = false;
  } finally { submit.disabled = false; }
});
byId("sign-out").addEventListener("click", async () => {
  try { await request("auth/logout", {}); location.reload(); }
  catch { message("Couldn’t sign out. Try again."); }
});
async function initialize() {
  try {
    accountLogin = (await request("auth/me")).login;
    byId("account-name").textContent = accountLogin;
    byId("account-name").hidden = !accountLogin;
    byId("account-open").hidden = Boolean(accountLogin);
    byId("sign-out").hidden = !accountLogin;
    if (accountLogin) byId("player-label").textContent = "Playing as";
    nick.readOnly = Boolean(accountLogin);
    nick.value = accountLogin;
  } catch { message("Couldn’t load your account. Refresh to try again."); }
  if (accountLogin) await pollRoom();
  if (!onlineRoom) await synchronize();
}
setupRooms();
initialize();
refreshScores();
