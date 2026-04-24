#include "poker/monte_carlo.hpp"

#include "poker/card.hpp"

#include <gtest/gtest.h>

#include <random>

TEST(MonteCarlo, AAPreflopEquityRough) {
    std::vector<poker::Card> aa = {poker::Card{12, 0}, poker::Card{12, 1}};
    std::vector<poker::Card> board;
    std::mt19937 rng{1234};
    const float eq = poker::simulate_hand_outcome(aa, board, 6000, rng, 1);
    EXPECT_GT(eq, 0.78F);
    EXPECT_LT(eq, 0.92F);
}

TEST(MonteCarlo, ParallelMatchesSingleThreadedRoughly) {
    std::vector<poker::Card> hole = {poker::Card{11, 0}, poker::Card{11, 1}};
    std::vector<poker::Card> board = {poker::Card{2, 0}, poker::Card{5, 1}, poker::Card{9, 2}};
    std::mt19937 rng{99};
    const float a = poker::simulate_hand_outcome(hole, board, 4000, rng, 1);
    const float b =
        poker::parallel_hand_simulation(hole, board, 4000, 99U, 1, 4);
    EXPECT_NEAR(a, b, 0.08F);
}
