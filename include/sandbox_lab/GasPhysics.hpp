#pragma once

#include "Grid.hpp"
#include "Movement.hpp"
#include <random>

namespace sandbox {
    class GasPhysics {
    public:
        GasPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection, std::uint64_t currentFrame);

        void updateGas(Grid &grid, int x, int y);
        void spawnSmoke(Grid &grid, int x, int y);
        void spawnSteam(Grid &grid, int x, int y);

    private:
        std::mt19937 &rng;
        std::uniform_int_distribution<int> &randomDirection;
        std::uint64_t currentFrame;

        bool moveGas(Grid &grid, int x, int y, MoveDirection direction);
    };
}
