#include "game_core.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    std::filesystem::path levelsDir = "levels";
    if (argc > 1) {
        levelsDir = argv[1];
    }

    xplat::GameCore game(1067, 600);
    if (!game.loadLevels(levelsDir)) {
        std::cerr << "Failed to load any levels from: " << levelsDir << "\n";
        return 1;
    }

    game.loadLevel(0);
    game.setGameState(xplat::GameState::Playing);

    xplat::InputState input;
    input.right = true;

    for (int frame = 0; frame < 300; ++frame) {
        game.update(1.0f / 60.0f, input);

        if (frame % 60 == 0) {
            const auto& p = game.playerRect();
            std::cout << "frame=" << frame
                      << " pos=(" << p.x << ',' << p.y << ")"
                      << " hp=" << game.playerHp()
                      << " attempts=" << game.attemptCount()
                      << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
