#pragma once

#include "poker/bot_config.hpp"
#include "poker/card.hpp"
#include "poker/game_state.hpp"
#include "poker/opponent_model.hpp"
#include "poker/types.hpp"

namespace poker {

/// Uses pot odds, optional Monte Carlo equity (`cfg.monte_carlo_simulations`), and `OpponentModel`.
[[nodiscard]] Decision decide_action(const PokerGameState& game_state,
                                     const std::vector<Card>& player_hand,
                                     const BotConfig& cfg,
                                     const OpponentModel* opponent_model = nullptr,
                                     int hero_seat = -1);

[[nodiscard]] double pot_odds_ratio(int pot, int to_call);

/// Simplified chip EV for calling once (fold = 0), ignores future streets.
[[nodiscard]] double expected_value_call(double equity, int pot, int to_call);

}  // namespace poker
