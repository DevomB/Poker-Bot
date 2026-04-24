#pragma once

#include "poker/card.hpp"
#include "poker/types.hpp"

#include <string>
#include <vector>

namespace poker {

struct Player {
    std::string name;
    std::vector<Card> hole_cards;
    int stack{0};
    int committed_this_street{0};
    int total_committed_hand{0};
    bool folded{false};
    int seat{0};
};

struct PokerGameState {
    std::vector<Player> players;
    std::vector<Card> community_cards;
    GamePhase phase{GamePhase::PreFlop};
    int pot{0};
    /// Maximum committed_this_street among active players this betting round.
    int current_bet{0};
    int button_seat{0};
    int small_blind{1};
    int big_blind{2};
    /// Seat index (into `players`) whose action is pending; -1 if none.
    int acting_index{-1};
    int last_raise_increment{0};
    /// First to act this betting street (seat order handled by engine).
    int street_opening_index{-1};
    /// Voluntary actions this street; reset on each street and on any raise.
    std::vector<bool> acted_this_street;

    [[nodiscard]] int to_call(const Player& p) const;
    [[nodiscard]] int active_players_in_hand() const;
};

/// Snapshot for strategy layer (hero-centric).
[[nodiscard]] BotView make_bot_view(const PokerGameState& state, int hero_seat);

}  // namespace poker
