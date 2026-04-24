#pragma once

#include "poker/card.hpp"

#include <cstddef>
#include <random>
#include <vector>

namespace poker {

/// Estimated equity (0–1) for hero winning share of a multi-way pot at showdown.
[[nodiscard]] float simulate_hand_outcome(const std::vector<Card>& player_hand,
                                          const std::vector<Card>& community_cards,
                                          int num_simulations,
                                          std::mt19937& rng,
                                          int villains = 1);

/// Same as repeated `simulate_hand_outcome`, split across `num_threads` workers.
[[nodiscard]] float parallel_hand_simulation(const std::vector<Card>& player_hand,
                                             const std::vector<Card>& community_cards,
                                             int num_simulations,
                                             std::uint32_t base_seed,
                                             int villains,
                                             std::size_t num_threads);

}  // namespace poker
