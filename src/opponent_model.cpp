#include "poker/opponent_model.hpp"

#include <algorithm>

namespace poker {

void OpponentModel::recompute_rates() {
    if (total_ == 0) {
        aggression_factor = 0.5F;
        call_frequency = 0.33F;
        fold_frequency = 0.33F;
        return;
    }
    const float inv = 1.0F / static_cast<float>(total_);
    call_frequency = static_cast<float>(calls_) * inv;
    fold_frequency = static_cast<float>(folds_) * inv;
    const float aggressive = static_cast<float>(raises_) * inv;
    aggression_factor = std::clamp(aggressive * 2.0F + 0.25F, 0.0F, 1.0F);
}

void OpponentModel::update_model(Action opponent_action) {
    ++total_;
    switch (opponent_action) {
        case Action::Raise:
            ++raises_;
            break;
        case Action::Call:
            ++calls_;
            break;
        case Action::Fold:
            ++folds_;
            break;
        case Action::Check:
            ++checks_;
            break;
    }
    recompute_rates();
}

}  // namespace poker
