# Poker Bot (NL Hold’em) — C++ library

A small **No-Limit Texas Hold’em** engine and decision helper in modern C++: deck and dealing, game state and betting streets, **7-card hand evaluation**, **Monte Carlo equity** (single- and multi-threaded), a **rule-based `decide_action`** layer (pot odds + simplified chip EV), optional **opponent frequency stats**, and a **`PokerBotInterface` hook** for simulators or other permitted integrations.

Use this for **your own simulator, research, or APIs you are allowed to automate**. It is not intended to help bypass third-party terms of service on real-money sites.

## Features

| Area | What’s included |
|------|------------------|
| **Cards / deck** | 52-card deck, shuffle with injected `std::mt19937`, deal, burn on board deals in `GameEngine` |
| **State & rules** | `PokerGameState`, blinds, pot, per-street commits, phase machine (pre-flop → river → showdown), `GameEngine::apply_action` with `Decision` |
| **Evaluation** | Best five of up to seven cards, full ranking + kickers, `evaluate_hand_strength` scalar |
| **Strategy** | `decide_action(..., BotConfig, OpponentModel*)` using MC equity (or strength fallback when sim count is 0), pot odds, and call EV |
| **Simulation** | `simulate_hand_outcome`, `parallel_hand_simulation` (chunked async workers, distinct seeds) |
| **Config** | `BotConfig::load_from_config_file` / `save_to_config_file` (`key=value`, `#` comments) |
| **Tests** | GoogleTest suite (deck, engine, evaluator, strategy, opponent model, MC, config) |

## Requirements

- **C++20** compiler (MSVC, GCC, or Clang)
- **CMake 3.16+**
- **Network** on first configure (GoogleTest is fetched with `FetchContent` when tests are enabled)

## Repository layout

```
include/poker/     Public headers (Card, Deck, GameEngine, HandEvaluator, …)
src/               Implementations
tests/             Unit tests
examples/          Sample program linking `poker_lib`
CMakeLists.txt     Library, optional `poker_example`, optional `poker_tests`
```

## Build

### Generic (single-configuration generators)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run tests:

```bash
cd build
ctest --output-on-failure
```

Run the example (if built):

```bash
./poker_example          # Unix
poker_example.exe        # Windows
```

### Windows: MSVC with NMake (typical when `cl` is not on PATH)

Open **x64 Native Tools** or run `vcvars64.bat`, then:

```bat
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -S . -B build_nmake
cmake --build build_nmake
cd build_nmake
ctest --output-on-failure
```

### CMake options

| Option | Default | Meaning |
|--------|---------|---------|
| `POKER_BUILD_TESTS` | `ON` | Build `poker_tests` and fetch GoogleTest |
| `POKER_BUILD_EXAMPLES` | `ON` | Build `poker_example` |

Disable tests (no GTest download):

```bash
cmake -S . -B build -DPOKER_BUILD_TESTS=OFF
```

### GCC note

Tests link `libstdc++fs` when using GNU C++ for `std::filesystem` in config tests.

## Configuration file

`BotConfig` reads simple line-based files:

```ini
# bot.txt
aggression_threshold=0.55
risk_tolerance=0.92
monte_carlo_simulations=800
monte_carlo_villains=1
raise_pot_fraction=0.55
opponent_aggression_weight=0.05
rng_seed=2463534242
```

Load with `BotConfig::load_from_config_file("bot.txt")`.

## Quick API sketch

- **State & engine**: `poker::PokerGameState`, `poker::GameEngine::start_new_hand`, `apply_action`, `advance_phase_if_ready`
- **Hands**: `poker::evaluate_best_hand`, `poker::evaluate_hand_strength`, `poker::evaluate_hand`
- **Equity**: `poker::simulate_hand_outcome`, `poker::parallel_hand_simulation`
- **Decision**: `poker::decide_action(state, hero_hole_cards, cfg, opponent_model, hero_seat)`
- **Integration**: subclass `poker::PokerBotInterface` or use `poker::MockPokerBotInterface` for tests

Headers live under `include/poker/`. Link against **`poker_lib`**.

## Example

See [`examples/simple_bot_main.cpp`](examples/simple_bot_main.cpp): starts a two-player hand, prints a `Decision`, and prints rough pre-flop MC equity.

## Contributing / tuning

Strategy thresholds and MC counts are intentionally centralized in **`BotConfig`**. Adjust and re-run **`ctest`** after changes; MC-heavy tests assume statistical bands (e.g. AA pre-flop equity vs one random hand).

## License

Specify your license in this repository (none is set in this README by default).
