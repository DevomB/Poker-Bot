#pragma once

#include "poker/game_state.hpp"
#include "poker/types.hpp"

#include <string>

namespace poker {

/// Integration boundary for simulators or permitted external hosts.
class PokerBotInterface {
public:
    virtual ~PokerBotInterface() = default;

    [[nodiscard]] virtual PokerGameState get_game_state() = 0;
    virtual void send_action(Decision decision) = 0;
};

/// In-memory stub for tests and offline play.
class MockPokerBotInterface final : public PokerBotInterface {
public:
    explicit MockPokerBotInterface(PokerGameState initial);

    [[nodiscard]] PokerGameState get_game_state() override;
    void send_action(Decision decision) override;

    void set_state(PokerGameState s);

    [[nodiscard]] std::string last_error() const { return last_error_; }

private:
    PokerGameState state_;
    std::string last_error_;
};

}  // namespace poker
