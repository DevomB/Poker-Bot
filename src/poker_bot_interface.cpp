#include "poker/poker_bot_interface.hpp"

namespace poker {

MockPokerBotInterface::MockPokerBotInterface(PokerGameState initial) : state_(std::move(initial)) {}

PokerGameState MockPokerBotInterface::get_game_state() { return state_; }

void MockPokerBotInterface::send_action(Decision decision) {
    last_error_.clear();
    (void)decision;
    // Stub: real integrations would forward to a host; state mutation lives in GameEngine.
}

void MockPokerBotInterface::set_state(PokerGameState s) { state_ = std::move(s); }

}  // namespace poker
