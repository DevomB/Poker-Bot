#pragma once

#include "poker/types.hpp"

#include <cstddef>

namespace poker {

class OpponentModel {
public:
    float aggression_factor{0.5F};
    float call_frequency{0.33F};
    float fold_frequency{0.33F};

    void update_model(Action opponent_action);

    [[nodiscard]] std::size_t samples() const { return total_; }

private:
    std::size_t raises_{0};
    std::size_t calls_{0};
    std::size_t folds_{0};
    std::size_t checks_{0};
    std::size_t total_{0};

    void recompute_rates();
};

}  // namespace poker
