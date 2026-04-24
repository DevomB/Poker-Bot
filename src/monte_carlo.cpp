#include "poker/monte_carlo.hpp"

#include "poker/hand_evaluator.hpp"

#include <algorithm>
#include <future>
#include <random>
#include <vector>

namespace poker {

namespace {

bool hand_ties(const HandEvaluation& a, const HandEvaluation& b) {
    return !(a < b) && !(b < a);
}

double hero_showdown_equity(const std::vector<HandEvaluation>& hands) {
    if (hands.empty()) {
        return 0.0;
    }
    HandEvaluation best = hands[0];
    for (const auto& h : hands) {
        if (best < h) {
            best = h;
        }
    }
    int tied_at_best = 0;
    bool hero_tied = false;
    for (std::size_t i = 0; i < hands.size(); ++i) {
        if (hand_ties(hands[i], best)) {
            ++tied_at_best;
            if (i == 0) {
                hero_tied = true;
            }
        }
    }
    if (!hero_tied || tied_at_best <= 0) {
        return 0.0;
    }
    return 1.0 / static_cast<double>(tied_at_best);
}

void collect_known(const std::vector<Card>& hero, const std::vector<Card>& board,
                   std::vector<bool>& used) {
    for (const auto& c : hero) {
        const int id = static_cast<int>(c.suit()) * 13 + static_cast<int>(c.rank());
        used[static_cast<std::size_t>(id)] = true;
    }
    for (const auto& c : board) {
        const int id = static_cast<int>(c.suit()) * 13 + static_cast<int>(c.rank());
        used[static_cast<std::size_t>(id)] = true;
    }
}

std::vector<Card> remaining_deck(const std::vector<bool>& used) {
    std::vector<Card> d;
    d.reserve(52);
    for (int s = 0; s < 4; ++s) {
        for (int r = 0; r < 13; ++r) {
            const int id = s * 13 + r;
            if (!used[static_cast<std::size_t>(id)]) {
                d.emplace_back(static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(s));
            }
        }
    }
    return d;
}

std::vector<Card> combine_hole_board(const std::vector<Card>& hole,
                                     const std::vector<Card>& runout) {
    std::vector<Card> all = hole;
    all.insert(all.end(), runout.begin(), runout.end());
    return all;
}

float run_chunk(const std::vector<Card>& player_hand, const std::vector<Card>& community_cards,
                int count, std::uint32_t seed, int villains) {
    if (count <= 0) {
        return 0.0F;
    }
    if (villains < 1) {
        villains = 1;
    }
    std::mt19937 rng(seed);
    std::vector<bool> known(52, false);
    collect_known(player_hand, community_cards, known);

    double equity_sum = 0.0;
    std::vector<Card> pool;
    std::vector<HandEvaluation> evals;
    evals.reserve(static_cast<std::size_t>(villains) + 1);

    for (int i = 0; i < count; ++i) {
        pool = remaining_deck(known);
        std::shuffle(pool.begin(), pool.end(), rng);

        std::size_t idx = 0;
        std::vector<std::vector<Card>> villain_holes(static_cast<std::size_t>(villains));
        for (auto& vh : villain_holes) {
            vh.push_back(pool[idx++]);
            vh.push_back(pool[idx++]);
        }

        std::vector<Card> runout = community_cards;
        while (runout.size() < 5) {
            runout.push_back(pool[idx++]);
        }

        evals.clear();
        evals.push_back(evaluate_best_hand(combine_hole_board(player_hand, runout)));
        for (const auto& vh : villain_holes) {
            evals.push_back(evaluate_best_hand(combine_hole_board(vh, runout)));
        }
        equity_sum += hero_showdown_equity(evals);
    }
    return static_cast<float>(equity_sum / static_cast<double>(count));
}

}  // namespace

float simulate_hand_outcome(const std::vector<Card>& player_hand,
                            const std::vector<Card>& community_cards, int num_simulations,
                            std::mt19937& rng, int villains) {
    if (num_simulations <= 0) {
        return 0.0F;
    }
    if (villains < 1) {
        villains = 1;
    }
    std::vector<bool> known(52, false);
    collect_known(player_hand, community_cards, known);

    double equity_sum = 0.0;
    std::vector<Card> pool;
    std::vector<HandEvaluation> evals;
    evals.reserve(static_cast<std::size_t>(villains) + 1);

    for (int i = 0; i < num_simulations; ++i) {
        pool = remaining_deck(known);
        std::shuffle(pool.begin(), pool.end(), rng);

        std::size_t idx = 0;
        std::vector<std::vector<Card>> villain_holes(static_cast<std::size_t>(villains));
        for (auto& vh : villain_holes) {
            vh.push_back(pool[idx++]);
            vh.push_back(pool[idx++]);
        }

        std::vector<Card> runout = community_cards;
        while (runout.size() < 5) {
            runout.push_back(pool[idx++]);
        }

        evals.clear();
        evals.push_back(evaluate_best_hand(combine_hole_board(player_hand, runout)));
        for (const auto& vh : villain_holes) {
            evals.push_back(evaluate_best_hand(combine_hole_board(vh, runout)));
        }
        equity_sum += hero_showdown_equity(evals);
    }
    return static_cast<float>(equity_sum / static_cast<double>(num_simulations));
}

float parallel_hand_simulation(const std::vector<Card>& player_hand,
                               const std::vector<Card>& community_cards, int num_simulations,
                               std::uint32_t base_seed, int villains, std::size_t num_threads) {
    if (num_simulations <= 0) {
        return 0.0F;
    }
    if (num_threads == 0) {
        num_threads = 1;
    }
    num_threads = std::min<std::size_t>(num_threads, static_cast<std::size_t>(num_simulations));
    const int base = num_simulations / static_cast<int>(num_threads);
    const int rem = num_simulations % static_cast<int>(num_threads);

    std::vector<std::future<float>> futures;
    futures.reserve(num_threads);
    for (std::size_t t = 0; t < num_threads; ++t) {
        const int chunk = base + (static_cast<int>(t) < rem ? 1 : 0);
        const std::uint32_t seed = base_seed + static_cast<std::uint32_t>(t) * 9743U;
        futures.push_back(std::async(std::launch::async, [&, chunk, seed]() {
            return run_chunk(player_hand, community_cards, chunk, seed, villains);
        }));
    }
    double weighted = 0.0;
    int total = 0;
    for (std::size_t t = 0; t < num_threads; ++t) {
        const int chunk = base + (static_cast<int>(t) < rem ? 1 : 0);
        weighted += static_cast<double>(futures[t].get()) * static_cast<double>(chunk);
        total += chunk;
    }
    if (total <= 0) {
        return 0.0F;
    }
    return static_cast<float>(weighted / static_cast<double>(total));
}

}  // namespace poker
