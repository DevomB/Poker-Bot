#include "poker/deck.hpp"

#include <algorithm>

namespace poker {

Deck::Deck() { reset_deck(); }

void Deck::reset_deck() {
    cards_.clear();
    cards_.reserve(52);
    for (std::uint8_t s = 0; s < 4; ++s) {
        for (std::uint8_t r = 0; r < 13; ++r) {
            cards_.emplace_back(r, s);
        }
    }
}

void Deck::shuffle(std::mt19937& rng) { std::shuffle(cards_.begin(), cards_.end(), rng); }

Card Deck::deal_card() {
    Card c = cards_.back();
    cards_.pop_back();
    return c;
}

}  // namespace poker
