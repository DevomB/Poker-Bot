#pragma once

#include "poker/card.hpp"

#include <random>
#include <vector>

namespace poker {

class Deck {
public:
    Deck();

    void reset_deck();
    void shuffle(std::mt19937& rng);

    /// Removes and returns top card. Undefined if empty — caller must check.
    Card deal_card();

    [[nodiscard]] bool empty() const { return cards_.empty(); }
    [[nodiscard]] std::size_t size() const { return cards_.size(); }

    [[nodiscard]] const std::vector<Card>& cards() const { return cards_; }

private:
    std::vector<Card> cards_;
};

}  // namespace poker
