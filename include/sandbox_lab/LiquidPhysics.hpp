#pragma once

#include "Grid.hpp"
#include "Movement.hpp"
#include <random>

namespace sandbox {
    class LiquidPhysics {
    public:
        LiquidPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection);

        void updateLiquid(Grid &grid, int x, int y, Material material);
        void displaceLiquidSideways(Grid &grid, int x, int y);

    private:
        std::mt19937 &rng;
        std::uniform_int_distribution<int> &randomDirection;
        static constexpr int horizontalFlowDistance = 3;

        float transferLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material, float amount);
        float balanceLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material);
        void spreadLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material);
        float displaceLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material, float amount);
        bool fallLiquid(Grid &grid, int x, int y, Material material);
    };
}
