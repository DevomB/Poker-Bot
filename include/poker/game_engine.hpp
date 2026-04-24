#pragma once

#include "poker/deck.hpp"
#include "poker/game_state.hpp"
#include "poker/types.hpp"

#include <random>

namespace poker {

struct GameEngineConfig {
    int small_blind{1};
    int big_blind{2};
    int button_seat{0};
};

/// Mutable engine: owns RNG and deck for dealing. Mutates `PokerGameState`.
class GameEngine {
public:
    explicit GameEngine(std::uint32_t seed = std::random_device{}());

    void start_new_hand(PokerGameState& state, const GameEngineConfig& cfg);
    void advance_phase_if_ready(PokerGameState& state);

    /// Applies a decision for `state.acting_index`. Returns false if illegal.
    bool apply_action(PokerGameState& state, Decision d);

    [[nodiscard]] bool betting_round_complete(const PokerGameState& state) const;

    std::mt19937& rng() { return rng_; }

private:
    void deal_hole_cards(PokerGameState& state);
    void post_blinds(PokerGameState& state);
    void reset_street_bets(PokerGameState& state);
    void deal_board_for_phase(PokerGameState& state);
    void set_next_actor(PokerGameState& state);
    [[nodiscard]] int first_preflop_actor(const PokerGameState& state) const;
    [[nodiscard]] int next_seat_after(int start, const PokerGameState& state) const;

    Deck deck_;
    std::mt19937 rng_;
};

}  // namespace poker
