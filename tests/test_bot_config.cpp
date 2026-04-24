#include "poker/bot_config.hpp"

#include <gtest/gtest.h>

#include <filesystem>

TEST(BotConfig, LoadSaveRoundTrip) {
    const std::filesystem::path p =
        std::filesystem::temp_directory_path() / "poker_bot_unit_test_cfg.txt";

    poker::BotConfig c;
    c.monte_carlo_simulations = 1337;
    c.aggression_threshold = 0.61F;
    ASSERT_TRUE(c.save_to_config_file(p.string()));

    poker::BotConfig c2;
    c2.load_from_config_file(p.string());
    EXPECT_EQ(c2.monte_carlo_simulations, 1337);
    EXPECT_FLOAT_EQ(c2.aggression_threshold, 0.61F);

    std::error_code ec;
    std::filesystem::remove(p, ec);
}
