#pragma once

#include <cstdint>
#include <string>

namespace poker {

/// Ranks: 0=2 .. 8=T, 9=J, 10=Q, 11=K, 12=A. Suits: 0-3 (cdhs order).
class Card {
public:
    Card() = default;
    Card(std::uint8_t rank, std::uint8_t suit);

    std::uint8_t rank() const { return rank_; }
    std::uint8_t suit() const { return suit_; }

    char rank_char() const;
    char suit_char() const;
    std::string to_string() const;

    bool operator==(const Card& o) const {
        return rank_ == o.rank_ && suit_ == o.suit_;
    }
    bool operator!=(const Card& o) const { return !(*this == o); }

private:
    std::uint8_t rank_{0};
    std::uint8_t suit_{0};
};

}  // namespace poker
