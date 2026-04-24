#include "poker/bot_config.hpp"

#include <cctype>
#include <fstream>
#include <sstream>

namespace poker {

namespace {

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
        s.erase(s.begin());
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.pop_back();
    }
    return s;
}

}  // namespace

void BotConfig::load_from_config_file(const std::string& file_path) {
    std::ifstream in(file_path);
    if (!in) {
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        if (key == "aggression_threshold") {
            aggression_threshold = std::stof(val);
        } else if (key == "risk_tolerance") {
            risk_tolerance = std::stof(val);
        } else if (key == "monte_carlo_simulations") {
            monte_carlo_simulations = std::stoi(val);
        } else if (key == "monte_carlo_villains") {
            monte_carlo_villains = std::stoi(val);
        } else if (key == "raise_pot_fraction") {
            raise_pot_fraction = std::stof(val);
        } else if (key == "opponent_aggression_weight") {
            opponent_aggression_weight = std::stof(val);
        } else if (key == "rng_seed") {
            rng_seed = static_cast<std::uint32_t>(std::stoul(val));
        }
    }
}

bool BotConfig::save_to_config_file(const std::string& file_path) const {
    std::ofstream out(file_path);
    if (!out) {
        return false;
    }
    out << "# poker_bot configuration\n";
    out << "aggression_threshold=" << aggression_threshold << '\n';
    out << "risk_tolerance=" << risk_tolerance << '\n';
    out << "monte_carlo_simulations=" << monte_carlo_simulations << '\n';
    out << "monte_carlo_villains=" << monte_carlo_villains << '\n';
    out << "raise_pot_fraction=" << raise_pot_fraction << '\n';
    out << "opponent_aggression_weight=" << opponent_aggression_weight << '\n';
    out << "rng_seed=" << rng_seed << '\n';
    return true;
}

}  // namespace poker
