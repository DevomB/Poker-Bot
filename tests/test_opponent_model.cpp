#include "poker/opponent_model.hpp"

#include <gtest/gtest.h>

TEST(OpponentModel, UpdatesFrequencies) {
    poker::OpponentModel m;
    m.update_model(poker::Action::Raise);
    m.update_model(poker::Action::Call);
    m.update_model(poker::Action::Fold);
    EXPECT_EQ(m.samples(), 3U);
    EXPECT_GT(m.aggression_factor, 0.0F);
}
