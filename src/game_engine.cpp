#include "poker/game_engine.hpp"

#include <algorithm>
#include <limits>

namespace poker {

namespace {

int max_committed_among_active(const PokerGameState& state) {
    int m = 0;
    for (const auto& pl : state.players) {
        if (!pl.folded) {
            m = std::max(m, pl.committed_this_street);
        }
    }
    return m;
}

bool player_can_act(const Player& p) { return !p.folded && p.stack > 0; }

}  // namespace

int PokerGameState::to_call(const Player& p) const {
    if (p.folded) {
        return 0;
    }
    return std::max(0, current_bet - p.committed_this_street);
}

int PokerGameState::active_players_in_hand() const {
    int n = 0;
    for (const auto& pl : players) {
        if (!pl.folded) {
            ++n;
        }
    }
    return n;
}

BotView make_bot_view(const PokerGameState& state, int hero_seat) {
    BotView v;
    v.hero_seat = hero_seat;
    v.pot = state.pot;
    v.big_blind = state.big_blind;
    const Player* hero = nullptr;
    for (const auto& p : state.players) {
        if (p.seat == hero_seat) {
            hero = &p;
            break;
        }
    }
    if (!hero) {
        return v;
    }
    v.hero_stack = hero->stack;
    v.to_call = state.to_call(*hero);
    v.can_check = v.to_call == 0;
    return v;
}

GameEngine::GameEngine(std::uint32_t seed) : rng_(seed) {}

void GameEngine::start_new_hand(PokerGameState& state, const GameEngineConfig& cfg) {
    std::sort(state.players.begin(), state.players.end(),
              [](const Player& a, const Player& b) { return a.seat < b.seat; });
    state.community_cards.clear();
    state.pot = 0;
    state.current_bet = 0;
    state.last_raise_increment = 0;
    state.acted_this_street.assign(state.players.size(), false);
    state.phase = GamePhase::PreFlop;
    state.small_blind = cfg.small_blind;
    state.big_blind = cfg.big_blind;
    state.button_seat = cfg.button_seat;

    for (auto& p : state.players) {
        p.hole_cards.clear();
        p.committed_this_street = 0;
        p.total_committed_hand = 0;
        p.folded = false;
        if (p.stack <= 0) {
            p.folded = true;
        }
    }

    deck_.reset_deck();
    deck_.shuffle(rng_);
    deal_hole_cards(state);
    post_blinds(state);
    set_next_actor(state);
}

void GameEngine::deal_hole_cards(PokerGameState& state) {
    for (int round = 0; round < 2; ++round) {
        for (auto& p : state.players) {
            if (p.stack > 0 && !p.folded) {
                p.hole_cards.push_back(deck_.deal_card());
            }
        }
    }
}

int GameEngine::next_seat_after(int start, const PokerGameState& state) const {
    if (state.players.empty()) {
        return -1;
    }
    int best = -1;
    int best_seat = std::numeric_limits<int>::max();
    for (std::size_t i = 0; i < state.players.size(); ++i) {
        const auto& p = state.players[i];
        if (p.stack <= 0) {
            continue;
        }
        if (p.seat > start && p.seat < best_seat) {
            best_seat = p.seat;
            best = static_cast<int>(i);
        }
    }
    if (best >= 0) {
        return best;
    }
    for (std::size_t i = 0; i < state.players.size(); ++i) {
        const auto& p = state.players[i];
        if (p.stack <= 0) {
            continue;
        }
        if (p.seat < best_seat) {
            best_seat = p.seat;
            best = static_cast<int>(i);
        }
    }
    return best;
}

int GameEngine::first_preflop_actor(const PokerGameState& state) const {
    int bb_idx = -1;
    for (std::size_t i = 0; i < state.players.size(); ++i) {
        if (state.players[i].committed_this_street == state.big_blind) {
            bb_idx = static_cast<int>(i);
            break;
        }
    }
    if (bb_idx < 0) {
        return 0;
    }
    const int bb_seat = state.players[static_cast<std::size_t>(bb_idx)].seat;
    return next_seat_after(bb_seat, state);
}

void GameEngine::post_blinds(PokerGameState& state) {
    int btn = state.button_seat;
    int sb_idx = next_seat_after(btn, state);
    int bb_idx = next_seat_after(state.players[static_cast<std::size_t>(sb_idx)].seat, state);

    auto pay = [&](int idx, int amt) {
        if (idx < 0) {
            return;
        }
        Player& p = state.players[static_cast<std::size_t>(idx)];
        int pay_amt = std::min(amt, p.stack);
        p.stack -= pay_amt;
        p.committed_this_street += pay_amt;
        p.total_committed_hand += pay_amt;
        state.pot += pay_amt;
    };

    pay(sb_idx, state.small_blind);
    pay(bb_idx, state.big_blind);

    int max_c = 0;
    for (const auto& p : state.players) {
        max_c = std::max(max_c, p.committed_this_street);
    }
    state.current_bet = max_c;
    state.last_raise_increment = state.big_blind;
    state.street_opening_index = first_preflop_actor(state);
    state.acted_this_street.assign(state.players.size(), false);
}

void GameEngine::reset_street_bets(PokerGameState& state) {
    for (auto& p : state.players) {
        p.committed_this_street = 0;
    }
    state.current_bet = 0;
    state.last_raise_increment = state.big_blind;
    state.acted_this_street.assign(state.players.size(), false);
    // First post-flop actor: next active seat after button.
    int start = state.button_seat;
    state.street_opening_index = next_seat_after(start, state);
}

void GameEngine::deal_board_for_phase(PokerGameState& state) {
    if (state.phase == GamePhase::Flop) {
        if (!deck_.empty()) {
            deck_.deal_card();
        }  // burn
        for (int i = 0; i < 3 && !deck_.empty(); ++i) {
            state.community_cards.push_back(deck_.deal_card());
        }
    } else if (state.phase == GamePhase::Turn || state.phase == GamePhase::River) {
        if (!deck_.empty()) {
            deck_.deal_card();
        }
        if (!deck_.empty()) {
            state.community_cards.push_back(deck_.deal_card());
        }
    }
}

void GameEngine::set_next_actor(PokerGameState& state) {
    const int n = static_cast<int>(state.players.size());
    if (n == 0) {
        state.acting_index = -1;
        return;
    }
    int open = state.street_opening_index;
    if (open < 0 || open >= n) {
        open = 0;
    }
    const int max_c = max_committed_among_active(state);
    for (int k = 0; k < n; ++k) {
        int idx = (open + k) % n;
        const Player& p = state.players[static_cast<std::size_t>(idx)];
        if (!player_can_act(p)) {
            continue;
        }
        if (p.committed_this_street < max_c) {
            state.acting_index = idx;
            return;
        }
    }
    for (int k = 0; k < n; ++k) {
        int idx = (open + k) % n;
        const Player& p = state.players[static_cast<std::size_t>(idx)];
        if (player_can_act(p) && !state.acted_this_street[static_cast<std::size_t>(idx)]) {
            state.acting_index = idx;
            return;
        }
    }
    state.acting_index = -1;
}

bool GameEngine::betting_round_complete(const PokerGameState& state) const {
    int max_commit = max_committed_among_active(state);
    for (std::size_t i = 0; i < state.players.size(); ++i) {
        const Player& p = state.players[i];
        if (p.folded) {
            continue;
        }
        if (player_can_act(p) && p.committed_this_street < max_commit) {
            return false;
        }
    }
    if (state.acted_this_street.size() != state.players.size()) {
        return false;
    }
    for (std::size_t i = 0; i < state.players.size(); ++i) {
        const Player& p = state.players[i];
        if (!player_can_act(p)) {
            continue;
        }
        if (!state.acted_this_street[i]) {
            return false;
        }
    }
    return true;
}

void GameEngine::advance_phase_if_ready(PokerGameState& state) {
    if (!betting_round_complete(state)) {
        return;
    }
    if (state.phase == GamePhase::Showdown || state.phase == GamePhase::HandComplete) {
        return;
    }
    reset_street_bets(state);
    if (state.phase == GamePhase::PreFlop) {
        state.phase = GamePhase::Flop;
        deal_board_for_phase(state);
    } else if (state.phase == GamePhase::Flop) {
        state.phase = GamePhase::Turn;
        deal_board_for_phase(state);
    } else if (state.phase == GamePhase::Turn) {
        state.phase = GamePhase::River;
        deal_board_for_phase(state);
    } else if (state.phase == GamePhase::River) {
        state.phase = GamePhase::Showdown;
        state.acting_index = -1;
        return;
    }
    set_next_actor(state);
}

bool GameEngine::apply_action(PokerGameState& state, Decision d) {
    if (state.acting_index < 0 ||
        state.acting_index >= static_cast<int>(state.players.size())) {
        return false;
    }
    if (state.phase == GamePhase::Showdown || state.phase == GamePhase::HandComplete) {
        return false;
    }
    const int ai = state.acting_index;
    Player& p = state.players[static_cast<std::size_t>(ai)];
    if (p.folded || p.stack <= 0) {
        return false;
    }
    if (state.acted_this_street.size() != state.players.size()) {
        state.acted_this_street.assign(state.players.size(), false);
    }

    int tc = state.to_call(p);

    switch (d.action) {
        case Action::Fold:
            p.folded = true;
            break;
        case Action::Check:
            if (tc != 0) {
                return false;
            }
            break;
        case Action::Call: {
            int pay = std::min(tc, p.stack);
            p.stack -= pay;
            p.committed_this_street += pay;
            p.total_committed_hand += pay;
            state.pot += pay;
            break;
        }
        case Action::Raise: {
            if (tc >= p.stack) {
                return false;
            }
            int call_part = std::min(tc, p.stack);
            int max_raise_by = p.stack - call_part;
            int inc = std::max(d.raise_by, state.last_raise_increment);
            inc = std::min(inc, max_raise_by);
            if (inc <= 0 && tc > 0) {
                return false;
            }
            int total_pay = std::min(call_part + inc, p.stack);
            int actual_inc = total_pay - call_part;
            if (actual_inc <= 0 && tc > 0) {
                return false;
            }
            p.stack -= total_pay;
            p.committed_this_street += total_pay;
            p.total_committed_hand += total_pay;
            state.pot += total_pay;
            int new_level = p.committed_this_street;
            if (new_level > state.current_bet) {
                state.last_raise_increment = std::max(state.big_blind, actual_inc);
                state.current_bet = new_level;
            }
            state.acted_this_street.assign(state.players.size(), false);
            state.acted_this_street[static_cast<std::size_t>(ai)] = true;
            break;
        }
    }

    if (d.action != Action::Raise) {
        state.acted_this_street[static_cast<std::size_t>(ai)] = true;
    }

    state.current_bet = max_committed_among_active(state);

    if (betting_round_complete(state)) {
        advance_phase_if_ready(state);
    } else {
        const int n = static_cast<int>(state.players.size());
        const int start = state.acting_index;
        const int max_c = max_committed_among_active(state);
        for (int step = 1; step <= n; ++step) {
            int idx = (start + step) % n;
            const Player& nx = state.players[static_cast<std::size_t>(idx)];
            if (!player_can_act(nx)) {
                continue;
            }
            if (nx.committed_this_street < max_c) {
                state.acting_index = idx;
                return true;
            }
        }
        for (int step = 1; step <= n; ++step) {
            int idx = (start + step) % n;
            const Player& nx = state.players[static_cast<std::size_t>(idx)];
            if (player_can_act(nx)) {
                state.acting_index = idx;
                return true;
            }
        }
        state.acting_index = -1;
    }
    return true;
}

}  // namespace poker
