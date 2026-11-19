"use strict";

const byId = (id) => document.getElementById(id);
const lobby = byId("lobby");
const gameArea = byId("game");
const notice = byId("notice");
const nick = byId("player-name");
const syncButton = byId("sync");

const suits = ["♣", "♦", "♥", "♠"];
const suitNames = ["трефы", "бубны", "червы", "пики"];
const figures = {11: "В", 12: "Д", 13: "К", 14: "Т"};
const faceNames = {11: "валет", 12: "дама", 13: "король", 14: "туз"};
const turnText = {
  attack: "Твой ход. Выбери карту для атаки.",
  defend: "Твоя защита. Покрой карту или возьми стол.",
  throw: "Подкинь карту или заверши раунд.",
  pickup: "Бот берёт. Подкинь карту или заверши раунд."
};
const resultText = {win: "Победа! Ты избавился от карт.", loss: "Поражение. Бот избавился от карт.", draw: "Ничья. Обе руки пусты."};

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
  return (faceNames[rank] ?? rank) + ", " + suitNames[id % 4];
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
  const lastTwo = count % 100;
  let word = "карт";
  if (lastTwo < 11 || lastTwo > 14) {
    if (count % 10 === 1) word = "карта";
    if (count % 10 >= 2 && count % 10 <= 4) word = "карты";
  }
  return count + " " + word;
}

function message(text) {
  notice.textContent = text;
  notice.hidden = !text;
}

function render() {
  lobby.hidden = Boolean(game) || needsSync;
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
  byId("opponent-count").textContent = countCards(game.bot_count);

  const table = byId("table");
  table.replaceChildren();
  if (game.table.length === 0) table.append(element("p", "empty", "Стол пуст"));
  for (const pair of game.table) {
    const cards = element("div", "table-pair");
    const attack = card(pair.attack, false);
    attack.setAttribute("aria-label", "Атака — " + cardName(pair.attack));
    cards.append(attack);
    if (pair.defense !== null) {
      const defense = card(pair.defense, false);
      defense.setAttribute("aria-label", "Отбой — " + cardName(pair.defense));
      cards.append(defense);
    }
    table.append(cards);
  }

  gameArea.setAttribute("aria-busy", String(waiting));
  const finished = game.phase === "finished";
  byId("turn").textContent = finished ? resultText[game.result] : turnText[game.phase];
  byId("hand-count").textContent = countCards(game.hand.length);
  byId("hand").replaceChildren(...game.hand.map((id) => card(id, true)));
  byId("take").hidden = !game.moves.includes(-1);
  byId("done").hidden = !game.moves.includes(-2);
  byId("done").textContent = game.phase === "pickup" ? "Завершить раунд" : "Бито";
  byId("again").hidden = !finished;
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
    message("Не удалось получить состояние стола. Проверь соединение и повтори запрос.");
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
      message(path === "games" ? "Сервер отклонил имя. Попробуй другой ник." : "Такой ход недоступен.");
    } else if (error.status === 429) {
      message("Слишком много запросов. Подожди немного.");
    } else {
      try {
        await loadGame();
        message(game ? "Показано текущее состояние партии." : "Активной партии нет. Можно начать новую.");
        if (game?.phase === "finished") refreshScores();
      } catch {
        needsSync = true;
        message("Связь со столом прервалась. Проверь состояние перед следующим ходом.");
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
  if (game?.moves.includes(move)) submit("moves", {move, version: game.version});
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
      return row;
    });
    byId("leaders").replaceChildren(...rows);
    byId("score-note").textContent = rows.length ? "" : "Здесь появятся результаты завершённых партий.";
    byId("score-note").hidden = Boolean(rows.length);
  } catch {
    if (requestId !== scoreRequest) return;
    byId("score-note").hidden = false;
    byId("score-note").textContent = "Не удалось обновить рейтинг.";
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
nick.addEventListener("input", () => nick.setCustomValidity(""));
byId("join").addEventListener("submit", (event) => {
  event.preventDefault();
  const name = nick.value.trim();
  if (!name) {
    nick.setCustomValidity("Введи имя.");
    nick.reportValidity();
    return;
  }
  submit("games", {name});
});
synchronize();
refreshScores();
