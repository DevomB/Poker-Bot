# poker-calculations — NL Hold’em engine & odds helpers (C++ core, Node addon)

A small **No-Limit Texas Hold’em** engine and decision helper in modern C++: deck and dealing, game state and betting streets, **7-card hand evaluation**, **Monte Carlo equity** (single- and multi-threaded), a **rule-based `decide_action`** layer (pot odds + simplified chip EV), optional **opponent frequency stats**, and a **`PokerBotInterface` hook** for simulators or other permitted integrations.

The same C++ code is published as the **`poker-calculations` npm package** (native addon via [node-addon-api](https://github.com/nodejs/node-addon-api) + [cmake-js](https://github.com/cmake-js/cmake-js)).

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

## NPM package (recommended)

**Requirements:** Node.js 18+, CMake 3.16+, and **MSVC with C++ tools** (e.g. Visual Studio Build Tools — workload “Desktop development with C++”). On Windows you can run `pnpm install` / `npm install` from normal PowerShell: `scripts/compile-addon.js` uses **vswhere** to put `cl.exe` / `nmake` on `PATH` and forces the **NMake Makefiles** generator so CMake does not rely on a stale Visual Studio solution under `build/`.

From the repo root:

```bash
pnpm install
# or: npm install
```

This runs `cmake-js` via `scripts/compile-addon.js`, which strips `node_modules` entries from `PATH` so a stray `rc` shim does not replace the Windows SDK resource compiler, sets **`CMAKE_RC_COMPILER`** when needed, and prepends the MSVC **Hostx64\\x64** toolchain directory.

**If configure still fails:** delete the `build` folder and retry. If vswhere cannot find MSVC, open **x64 Native Tools Command Prompt for VS** and run install again, or install the **VC Tools** component.

**Develop / rebuild addon**

```bash
npm run build
```

**JavaScript API** (see [`index.d.ts`](index.d.ts)):

- **Native:** `evaluateBestHand`, `evaluateHandStrength`, `evaluateHandCategory`, `simulateHandOutcome`, `parallelHandSimulation`, `decideAction`, `potOddsRatio`, `expectedValueCall`.
- **Pure JS (`poker-math.js`):** `spr`, `effectiveStack`, `breakevenCallEquity`, `minimumDefenseFrequency`, `stackInBigBlinds`, `potOddsRatioDisplay`, `formatPotOdds`, `ruleOfFourEquity`, `ruleOfTwoEquity`, `impliedBreakevenFutureWin`, `bluffToValueRatio`.

Cards are strings such as `"Ah"` or `"Td"`; optional `"10h"` for ten-high.

**Example**

```bash
node examples/demo.mjs
```

**Native C++ tests only** (separate CMake build dir `build_native_tests`, no Node addon):

```bash
npm test
```

On Windows this expects the same MSVC-on-PATH setup as above.

## Plain CMake (library only)

### Repository layout

```
include/poker/     Public headers (Card, Deck, GameEngine, HandEvaluator, …)
src/               Implementations
native/            Node-API binding (built when CMAKE_JS_INC is set by cmake-js)
poker-math.js      Chip / odds formulas (SPR, MDF, rule of 2 & 4, …)
tests/             Unit tests
examples/          demo.mjs (Node)
CMakeLists.txt     Static poker_lib; optional poker_tests; optional poker_calculations.node when built by cmake-js
```

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
| `POKER_BUILD_TESTS` | `ON` when not using cmake-js; **`OFF`** when `CMAKE_JS_INC` is set (npm install path) | Build `poker_tests` and fetch GoogleTest |

When **cmake-js** configures the project, it defines `CMAKE_JS_INC` and only **`poker_calculations.node`** plus **`poker_lib`** are built—tests are skipped so `npm install` does not download GoogleTest.

Disable tests manually:

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

## Quick C++ API sketch

- **State & engine**: `poker::PokerGameState`, `poker::GameEngine::start_new_hand`, `apply_action`, `advance_phase_if_ready`
- **Hands**: `poker::evaluate_best_hand`, `poker::evaluate_hand_strength`, `poker::evaluate_hand`
- **Equity**: `poker::simulate_hand_outcome`, `poker::parallel_hand_simulation`
- **Decision**: `poker::decide_action(state, hero_hole_cards, cfg, opponent_model, hero_seat)`
- **Integration**: subclass `poker::PokerBotInterface` or use `poker::MockPokerBotInterface` for tests

Headers live under `include/poker/`. Link against **`poker_lib`**.

## Contributing / tuning

Strategy thresholds and MC counts are intentionally centralized in **`BotConfig`**. Adjust and re-run **`npm test`** or **`ctest`** after changes; MC-heavy tests assume statistical bands (e.g. AA pre-flop equity vs one random hand).

## License

Specify your license in this repository (none is set in this README by default).
