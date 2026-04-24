#include "poker/card.hpp"

#include <stdexcept>

namespace poker {

namespace {

constexpr const char* kRanks = "23456789TJQKA";

}  // namespace

Card::Card(std::uint8_t rank, std::uint8_t suit) : rank_(rank), suit_(suit) {
    if (rank > 12) {
        throw std::out_of_range("Card rank must be 0..12");
    }
    if (suit > 3) {
        throw std::out_of_range("Card suit must be 0..3");
    }
}

char Card::rank_char() const { return kRanks[rank_]; }

char Card::suit_char() const {
    static constexpr char kSuits[] = {'c', 'd', 'h', 's'};
    return kSuits[suit_];
}

std::string Card::to_string() const {
    return std::string{rank_char(), suit_char()};
}

}  // namespace poker
