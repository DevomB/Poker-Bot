#pragma once

#include <cstdint>
#include <string>

namespace poker {

class BotConfig {
public:
    float aggression_threshold{0.55F};
    float risk_tolerance{0.92F};
    int monte_carlo_simulations{800};
    int monte_carlo_villains{1};
    float raise_pot_fraction{0.55F};
    float opponent_aggression_weight{0.05F};
    std::uint32_t rng_seed{2463534242U};

    void load_from_config_file(const std::string& file_path);
    [[nodiscard]] bool save_to_config_file(const std::string& file_path) const;
};

}  // namespace poker
