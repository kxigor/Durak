# Durak

A game engine for **Durak** (подкидной дурак), written in modern **C++23**. The engine is
rule-driven: the flow of a game is expressed as small, injectable rule objects rather than
hard-coded logic, which keeps the core engine deterministic and the game variants pluggable.

> Status: the engine core and the setup/round rules are implemented and unit-tested. The
> per-move action rules (attack / toss / beat) and AI players are the next milestone — see
> [Roadmap](#roadmap).

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
app/main.cpp     minimal driver wiring the engine together
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

## Roadmap

- Concrete `ActionRule`s for the open / toss / beat phases (the phases run today, but the action
  sets are still stubs, so the table does not yet fill during real play).
- AI players (`Player::get_action`) and a CLI front-end (`include/durak/ui/cli`).

## License

Source-available under a custom license — you may view, build, and play. See [`LICENSE`](LICENSE).
