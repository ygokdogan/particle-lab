#pragma once

#include "Grid.hpp"
#include "Movement.hpp"
#include <random>

namespace sandbox {
    class LiquidPhysics;

    class PowderPhysics {
    public:
        PowderPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection, std::uint64_t &currentFrame, LiquidPhysics &liquidPhysics);

        void updatePowder(Grid &grid, int x, int y);

    private:
        std::mt19937 &rng;
        std::uniform_int_distribution<int> &randomDirection;
        std::uint64_t& currentFrame;
        LiquidPhysics &liquidPhysics;

        bool canMoveSolid(Grid &grid, int x, int y, MoveDirection direction) const;
        bool moveSolid(Grid &grid, int x, int y, MoveDirection direction);
    };
}
