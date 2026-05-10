# poker-calculations — NL Hold’em engine & odds helpers (C++ core, Node addon)

A small **No-Limit Texas Hold’em** engine and decision helper in modern C++: deck and dealing, game state and betting streets, **7-card hand evaluation**, **Monte Carlo equity** (single- and multi-threaded), a **rule-based `decide_action`** layer (pot odds + simplified chip EV), optional **opponent frequency stats**, and a **`PokerBotInterface` hook** for simulators or other permitted integrations.

The same C++ code is published as the **`poker-calculations` npm package**: the implementation stays in **C++**, but **published releases ship prebuilt native binaries** ([`node-gyp-build`](https://github.com/prebuild/node-gyp-build)) so **`npm install` does not require CMake, a compiler, or the Windows SDK** on the downloader’s machine.

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

## NPM package

### Installing from the registry (typical consumer)

**Requirements:** Node.js 18+ only. The tarball includes **prebuilt** `prebuilds/<platform>-<arch>/node.napi.node` files; [`node-gyp-build`](https://github.com/prebuild/node-gyp-build) picks the right one at load time.

```bash
npm install poker-calculations
```

No CMake, compiler, or SDK install is required.

### Cloning this repo or installing from Git (developers)

Native code is **not** compiled during `npm install` for published packages. If you clone the repository or install from a git URL before prebuilds have been generated locally, you need a normal **C++ toolchain** (CMake 3.16+, and MSVC with C++ workload on Windows, Xcode CLI tools on macOS, or GCC on Linux), then:

```bash
npm ci
npm run build:native
node scripts/stage-prebuild.js <platform-arch>
```

Use the same `<platform-arch>` tuple [`node-gyp-build` expects](https://github.com/prebuild/node-gyp-build) (for example `win32-x64`, `linux-x64`, `darwin-arm64`). On Windows, delete a stale `build` folder if configure fails; ensure the **Windows SDK** is installed if you see resource-compiler (`rc`) or manifest (`mt`) errors.

### Automated npm releases (GitHub Actions)

Publishing uses workflow [`.github/workflows/native-prebuild.yml`](.github/workflows/native-prebuild.yml):

1. Add a repository secret **`NPM_TOKEN`** (npmjs.com → Access Tokens → classic **Automation** token, or a granular token with publish rights for `poker-calculations`).
2. Bump **`package.json`** `version` (e.g. `1.0.1`) and push to **`main`**.

The workflow runs when **`package.json` changes** on `main`. It asks the registry whether that **`name@version` already exists**; if not, it builds all four native targets, merges them into `prebuilds/*/node.napi.node`, runs **`npm publish --provenance`**, and uploads the tarball to the public npm registry. **No git tags** — the version field is the only release input.

You can also run the workflow manually from the Actions tab (**workflow_dispatch**). If the current `package.json` version is already published, the workflow skips build and publish.

If publish fails on **`--provenance`** (registry or npm version), edit the workflow step to plain **`npm publish --access public`** (same secret).

**You do not need a self-hosted server** for this flow; GitHub-hosted runners are enough. A custom runner would only matter if you wanted builds somewhere without GitHub’s matrix (not required here).

**Manual publish** (same checks): assemble binaries under `prebuilds/`, then `npm publish`. Without binaries, `prepack` fails unless `SKIP_PREBUILD_CHECK=1`.

### Rebuild native addon (after changing C++)

```bash
npm run build:native
```

### How you call this code (programmatic API — not HTTP)

This package is a **Node.js library**: you `require()` or `import` it and call functions. There are **no REST endpoints** unless you wrap it in your own server.

**CommonJS**

```js
const poker = require('poker-calculations');

poker.evaluateBestHand(['Ah', 'Ac', 'Kd', 'Ks', 'Qh']); // native — best 5 of 7
const equity = poker.simulateHandOutcome(['Ah', 'Kh'], ['Qh', 'Jh', 'Th'], 2000, 42, 1);
const math = poker.spr(90, 270); // pure JS
```

**ESM**

```js
import { createRequire } from 'module';
const require = createRequire(import.meta.url);
const poker = require('poker-calculations');
```

Reference: **[`index.d.ts`](index.d.ts)** (every export and type). Runnable sample: **[`examples/demo.mjs`](examples/demo.mjs)** (`node examples/demo.mjs` from repo root).

| Layer | Role |
|--------|------|
| **Native (C++ via N-API)** | `evaluateBestHand`, `evaluateHandStrength`, `evaluateHandCategory`, `simulateHandOutcome`, `parallelHandSimulation`, `decideAction`, `potOddsRatio`, `expectedValueCall` |
| **Pure JS** (`poker-math.js`) | `spr`, `effectiveStack`, `breakevenCallEquity`, `minimumDefenseFrequency`, `stackInBigBlinds`, `potOddsRatioDisplay`, `formatPotOdds`, `ruleOfFourEquity`, `ruleOfTwoEquity`, `impliedBreakevenFutureWin`, `bluffToValueRatio` |

Cards are strings like `"Ah"`, `"Td"` (ten may be `"10h"`).

```bash
node examples/demo.mjs
```

### Native C++ tests (GoogleTest)

Separate CMake build dir `build_native_tests` (no Node addon):

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

When **cmake-js** configures the project, it defines `CMAKE_JS_INC` and only **`poker_calculations.node`** plus **`poker_lib`** are built—tests are skipped so the addon build stays fast and does not pull GoogleTest.

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
