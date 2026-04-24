#pragma once

#include <cstdint>
#include <string>

namespace poker {

enum class GamePhase {
    PreFlop,
    Flop,
    Turn,
    River,
    Showdown,
    HandComplete,
};

enum class Action { Fold, Call, Raise, Check };

enum class HandRank {
    HighCard,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
    RoyalFlush,
};

/// For Raise, `raise_by` is chips added on top of the amount needed to call
/// (increment above the call). For Check/Call/Fold it is ignored.
struct Decision {
    Action action{Action::Fold};
    int raise_by{0};
};

struct BotView {
    int hero_seat{0};
    int pot{0};
    int to_call{0};
    int big_blind{0};
    int hero_stack{0};
    bool can_check{false};
};

}  // namespace poker
