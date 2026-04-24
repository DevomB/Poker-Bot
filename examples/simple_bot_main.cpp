#include "poker/bot_config.hpp"
#include "poker/card.hpp"
#include "poker/game_engine.hpp"
#include "poker/hand_evaluator.hpp"
#include "poker/monte_carlo.hpp"
#include "poker/poker_bot_interface.hpp"
#include "poker/strategy.hpp"

#include <iostream>
#include <random>

int main() {
    poker::PokerGameState state;
    state.players.push_back({"Hero", {}, 200, 0, 0, false, 0});
    state.players.push_back({"Villain", {}, 200, 0, 0, false, 1});

    poker::GameEngine engine{12345};
    engine.start_new_hand(state, poker::GameEngineConfig{1, 2, 0});

    poker::BotConfig cfg;
    cfg.monte_carlo_simulations = 400;
    cfg.rng_seed = 42;

    const auto& hero = state.players[0];
    const poker::Decision d =
        poker::decide_action(state, hero.hole_cards, cfg, nullptr, hero.seat);

    std::cout << "Hero hole: ";
    for (const auto& c : hero.hole_cards) {
        std::cout << c.to_string() << ' ';
    }
    std::cout << "\nDecision action: " << static_cast<int>(d.action)
              << " raise_by: " << d.raise_by << '\n';

    std::mt19937 rng{99};
    const float eq = poker::simulate_hand_outcome(hero.hole_cards, state.community_cards, 2000, rng,
                                                  1);
    std::cout << "Preflop MC equity vs 1 random hand (approx): " << eq << '\n';

    const auto ev = poker::evaluate_best_hand(hero.hole_cards);
    std::cout << "Preflop category (2-card high): " << static_cast<int>(ev.rank) << '\n';

    poker::MockPokerBotInterface iface{state};
    (void)iface.get_game_state();
    iface.send_action(d);
    return 0;
}
