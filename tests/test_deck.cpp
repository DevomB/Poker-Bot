#include "poker/deck.hpp"

#include <gtest/gtest.h>

#include <random>

TEST(Deck, DealsFiftyTwoUnique) {
    poker::Deck d;
    std::uint64_t mask = 0;
    for (int i = 0; i < 52; ++i) {
        poker::Card c = d.deal_card();
        const int id = static_cast<int>(c.suit()) * 13 + static_cast<int>(c.rank());
        ASSERT_GE(id, 0);
        ASSERT_LT(id, 52);
        const std::uint64_t bit = 1ULL << id;
        ASSERT_EQ(mask & bit, 0ULL);
        mask |= bit;
    }
    EXPECT_TRUE(d.empty());
    EXPECT_EQ(mask, (1ULL << 52) - 1);
}

TEST(Deck, ShuffleWithDifferentSeeds) {
    poker::Deck a;
    poker::Deck b;
    std::mt19937 r1{1};
    std::mt19937 r2{2};
    a.shuffle(r1);
    b.shuffle(r2);
    bool diff = false;
    for (std::size_t i = 0; i < a.cards().size(); ++i) {
        const auto& ca = a.cards()[i];
        const auto& cb = b.cards()[i];
        if (ca.rank() != cb.rank() || ca.suit() != cb.suit()) {
            diff = true;
            break;
        }
    }
    EXPECT_TRUE(diff);
}
