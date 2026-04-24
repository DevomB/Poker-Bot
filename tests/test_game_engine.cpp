#include "poker/game_engine.hpp"

#include <gtest/gtest.h>

TEST(GameEngine, StartsHandWithHoleCardsAndBlinds) {
    poker::PokerGameState state;
    state.players.push_back({"A", {}, 100, 0, 0, false, 0});
    state.players.push_back({"B", {}, 100, 0, 0, false, 1});

    poker::GameEngine engine{999};
    engine.start_new_hand(state, poker::GameEngineConfig{1, 2, 0});

    EXPECT_EQ(state.phase, poker::GamePhase::PreFlop);
    EXPECT_EQ(state.players[0].hole_cards.size(), 2U);
    EXPECT_EQ(state.players[1].hole_cards.size(), 2U);
    EXPECT_GE(state.pot, 3);
    EXPECT_GE(state.acting_index, 0);
}

TEST(GameEngine, FoldEndsWithOneActive) {
    poker::PokerGameState state;
    state.players.push_back({"A", {}, 100, 0, 0, false, 0});
    state.players.push_back({"B", {}, 100, 0, 0, false, 1});

    poker::GameEngine engine{1001};
    engine.start_new_hand(state, poker::GameEngineConfig{1, 2, 0});
    const int actor = state.acting_index;
    poker::Decision fold{poker::Action::Fold, 0};
    ASSERT_TRUE(engine.apply_action(state, fold));
    EXPECT_TRUE(state.players[static_cast<std::size_t>(actor)].folded);
}
