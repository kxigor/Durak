# Durak

A game engine for **Durak** (подкидной дурак), written in modern **C++23**. The engine is
rule-driven: the flow of a game is expressed as small, injectable rule objects rather than
hard-coded logic, which keeps the core engine deterministic and the game variants pluggable.

## Play the web game

The browser game is a 36-card, two-player match against a server-side bot.
Start it locally with Docker Compose v2 or later:

```sh
docker compose up -d --build --wait
```

Open **http://localhost:8189**. NGINX serves the board and styles and proxies
`/api/` to the C++ server. The server validates each move, runs the bot and
returns the human hand, visible cards and legal moves. A refresh restores the
current game using a host-only HttpOnly cookie. Active games live in memory for
up to two hours of inactivity and end on a backend restart. Completed results
are recorded once in PostgreSQL on the `pgdata` volume. The `init` service
creates the schema and seeds Igor and Pavel before the API starts. Re-running
it preserves users and scores. The leaderboard totals results by display name. Guest nicknames are shared. Register a username and password to
earn scores under a verified identity (✓). Guest scores stay separate, including
scores recorded before registration. Usernames are case-insensitive. Signing in
rotates the seven-day account session. Signing out revokes it on the server.
Passwords are hashed using Argon2id. There is no email or password recovery flow.

```sh
curl --fail http://localhost:8189/api/health
curl --fail http://localhost:8189/api/leaderboard
docker compose down
```

Only NGINX publishes a port, bound to host loopback. In the standard Docker
configuration its ingress bridge allows native Docker to forward the port. The
API uses the internal game network. PostgreSQL listens on a Unix socket with
peer authentication and has its network disabled. The API runs as the restricted
database role `nobody`; the initializer runs as `postgres`. The database uses
a random bootstrap password and the initializer clears it. Game containers
drop capabilities, run as non-root and use read-only root filesystems.

On the dedicated **game VPS** run the same Compose services on rootless Podman
with `-f compose.yaml -f deploy/compose.isolated.yaml`. This removes the
frontend's internet-facing bridge. Rootless Podman still publishes the loopback
port from its internal network. Terminate HTTPS on the host and proxy to
`127.0.0.1:8189`. Keep this VPS separate from other private infrastructure.

### Automatic releases

GitHub Actions runs the unit suites and plays a full game through NGINX. For a
successful push to `main`, the workflow publishes images tagged with the commit
SHA to GHCR. Make the two packages public for anonymous image pulls.
The build and test jobs need no VPS credentials. The publish step uses GitHub’s
short-lived job token for GHCR. The release contains application code and
static assets. Keep TLS private keys and runtime databases on the game host.

Install Gitleaks and run `git config --local core.hooksPath .githooks` in a new
checkout to check outgoing commits before they leave the machine. CI also
scans Git history with redacted output.

The game VPS checks GitHub for a successful release every five minutes and
pulls that SHA. GitHub has no SSH key for the VPS. The deploy script, Compose
configuration and systemd user units are installed once by its administrator.
Only application images are updated automatically. Before deploying a release with a
schema change, install its `app/database/init.sql` on the host. The existing init
container applies the idempotent migration before the backend starts. Keep local
storage overrides when updating the operator files.

On the Ubuntu game VPS install Podman, docker-compose-v2, uidmap, passt, aardvark-dns,
dbus-user-session, curl and jq. Enable lingering for the dedicated game
account. As that user install the deployment files and start the socket:

```sh
install -d -m 700 ~/durak/deploy ~/durak/app/database ~/.config/systemd/user
install -m 600 compose.yaml ~/durak/compose.yaml
install -m 600 deploy/compose.isolated.yaml ~/durak/deploy/
install -m 644 app/database/init.sql ~/durak/app/database/
install -m 700 deploy/update.sh ~/durak/update.sh
install -m 600 deploy/durak-update.{service,timer} ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable --now podman.socket durak-update.timer
```

The updater checks the API through the frontend and restores the
previous image tag after an unsuccessful update. Keep the Compose project name
and volume across releases. To verify the full game locally in a disposable
Compose project, run `python3 tests/web_game.py http://localhost:8189`. This
check adds one completed game to its leaderboard.

## Architecture

The engine never decides *what* the rules of Durak are; it only decides *when* to ask a rule to
act. All game logic lives in rule objects that mutate the board through a narrow view.

### Building blocks

| Component | Responsibility |
|:---|:---|
| `DurakEngine` | Owns the board (players, stock, bita, trump, table, seating) and runs the game loop. Rules and action sets are injected via its constructor. |
| `GameRule` | A phase rule: `apply(GameRuleView&)` mutates the board at a lifecycle point (deck init, first turn, round start/end). |
| `ActionRule` | A player-move rule: `is_available` (verb usable now) + `is_legal` (this operand allowed) + `apply`. Grouped into open / toss / beat action sets. |
| `Move` | A polymorphic, engine-opaque player move carrying its `action_id`; concrete moves live in the rules layer and are validated by their `ActionRule`. |
| `GameRuleView` | The read/write handle a `GameRule` gets: `players()`, `table()`, `stock()`, `bita()`, `trump()`, attacker/defender positions, `advance_seat()`. |
| `PlayerView` | The read-only handle a `Player` gets to decide a move (visible counts, trump, positions, offset, legal actions). |
| `Player` | Abstract holder of a hand; `get_action(PlayerView) -> move_t` (`nullptr` = pass). |
| `Table` / `CardOnTable` | The cards in play this bout: each attack card plus an optional beat card (`is_beaten()`). |
| `Card` / `CardFace` | `Card` is an opaque `card_id`; `CardFace` encodes/decodes it into `Suit` + `Rank`. |
| `ModuloRing` | Seating arithmetic on a ring sized to the player count; `next_offset` must be coprime with it. |

### Roles and seating

Attacker and defender are **positions** on the seating ring, not flags on a player. Only rules
move them, through `GameRuleView::set_attacker_pos` / `set_defender_pos`. The handoff is split so
each rule owns one decision:

- `FirstTurnRule` sets the **opening attacker**.
- `RoundStartRule` derives the **defender** (next active seat after the attacker) each round.
- `RoundEndRule` sets the **next attacker** based on the bout's outcome.

### Game lifecycle

```
start(players)
  ├─ DeckInitRule      build + shuffle stock, reveal trump (bottom card)
  ├─ FirstTurnRule     deal 6 to each player, pick opening attacker (lowest trump)
  └─ game loop  ── while more than one player holds cards ──
        ├─ RoundStartRule   defender := next active seat after attacker
        ├─ open  phase      attacker plays
        ├─ toss  phase       everyone but the defender may toss
        ├─ beat  phase       defender beats or gives up
        └─ RoundEndRule     resolve table (beaten → bita, else defender takes all)
                            → refill hands to 6 (attacker first, defender last)
                            → next attacker (defended → defender; taken → skip defender)
  └─ loser = the last player still holding cards (the "durak")
```

### The implemented rules

| Rule | id | Job |
|:---|:--:|:---|
| `DeckInitRule` | 1 | Build the stock for the chosen `DeckKind`, shuffle, reveal the trump (the bottom card; cards are drawn from the top). |
| `FirstTurnRule` | 2 | Deal `kHandSize` (6) cards to every player, then set the opening attacker to the holder of the lowest trump. |
| `RoundStartRule` | 3 | Set the defender to the next card-holding seat after the attacker. |
| `RoundEndRule` | 4 | Sweep the table (all beaten → *bita*, otherwise the defender picks everything up), refill hands to 6, then advance the attacker. |

A `DeckKind` (`include/durak/rules/card_face.hpp`) captures a deck by its **lowest rank** (the
top is always the Ace) plus a jokers flag; the card count follows from that. Presets:
`deck::kPiquet` (24), `deck::kSkat` (32), `deck::kThirtySix` (36, default), `deck::kStandard`
(52), `deck::kJokers` (54).

## Project layout

```
include/durak/
  engine/        core: engine, game_rule, action_rule, move, game_rule_view,
                 player, player_view, table, card, modulo, config
  rules/         card_face, deck_init, first_turn, round_start, round_end,
                 turn_order, game_constants
  ui/cli/        cli.hpp (placeholder for a future CLI front-end)
src/durak/       engine/ and rules/ implementations
app/demo/        example driver (durak_app)
app/server/      HTTP API, PostgreSQL leaderboard and backend Dockerfile
app/database/    schema and idempotent seed data
app/web/         browser UI, NGINX and frontend Dockerfile
deploy/          updater and systemd user units for the game VPS
tests/unit/      basic/ · engine/ · rules/  (GTest)
cmake/           ProjectOptions, Sanitizers, Coverage, Format/Tidy tooling
```

## Conventions

- **C++23**, warnings-as-errors, and a strict `clang-tidy` profile (`.clang-tidy`) enforced as a
  build-gating CTest test.
- Naming is `snake_case`, with **constants in `k`+CamelCase** — including `const` value locals
  (`kRuleId`, `kHandSize`, `kLo`). `const` *reference* locals stay `snake_case`.
- Code carries no incidental comments; structure is expressed through small, named helpers.

## Prerequisites

- **CMake** ≥ 3.25 and **Ninja**
- A **C++23** compiler (GCC 14+ / Clang 18+)
- **LLVM tools**: `clang-format`, `clang-tidy` (for the QA checks)
- **Coverage** (optional): `gcov`, `lcov`, `genhtml`

## Build and run

The build is driven by `CMakePresets.json`. List everything with `cmake --list-presets`.

The default development preset is a Debug build with the **Address Sanitizer**:

```shell
cmake --preset dev-debug-asan
cmake --build --preset dev-debug-asan
./build/dev-debug-asan/app/durak_app
```

Other sanitizer presets: `dev-debug-tsan` (threads), `dev-debug-msan` (uninitialized reads),
`dev-debug-ubsan` (undefined behavior). For an optimized build use `cmake --preset ci-release`.

## Testing and quality

`ctest` runs the format check, the tidy check, and every unit suite together — so a green
`ctest` means formatted, lint-clean, and passing:

```shell
cmake --preset dev-debug-asan
cmake --build --preset dev-debug-asan
ctest --preset dev-debug-asan
```

Filter suites with a regex, e.g. only the rules tests:

```shell
ctest --preset dev-debug-asan -R 'DeckInit|FirstTurn|RoundStart|RoundEnd'
```

Individual QA checks are also available as `FormatCheck` / `TidyCheck` CTest tests. New GTest
suites are registered with the `add_gtest(...)` helper (see any `tests/unit/*/CMakeLists.txt`).

### Coverage

```shell
cmake --preset ci-coverage
cmake --build --preset ci-coverage --target coverage
```

The HTML report lands at `build/ci-coverage/coverage_report/index.html`.

## Game modes

The web game uses the step-based `rules::Match` mode with the existing deck,
deal and round rules. The generic synchronous `DurakEngine::start` driver and
its injectable action sets remain available for other game variants.

## License

Source-available under a custom license — you may view, build, and play. See [`LICENSE`](LICENSE).
