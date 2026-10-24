#pragma once

#include <random>
#include "Grid.hpp"

namespace sandbox {
    class Simulation {
        public:
            void update(Grid& grid);

        private: 
            enum class MoveDirection {Down, DownLeft, DownRight, Left, Right, UpLeft, UpRight, Up};

            std::mt19937 rng;
            std::uniform_int_distribution<int> randomDirection{0, 1};
            
            std::uint64_t currentFrame;
            bool scanLeftToRight = true;
            
            std::pair<int, int> getTargetPosition(int x, int y, MoveDirection direction) const;
            bool canMoveSolid(Grid& grid, int x, int y, MoveDirection direction) const;
            bool moveSolid(Grid& grid, int x, int y, MoveDirection direction);
            
            static constexpr int horizontalFlowDistance = 3;
            float transferLiquid(Grid& grid, int x, int y, MoveDirection direction, Material material, float amount);
            float balanceLiquid(Grid& grid, int x, int y, MoveDirection direction, Material material);
            void spreadLiquid(Grid& grid, int x, int y, MoveDirection direction, Material material);
            float displaceLiquid(Grid& grid, int x, int y, MoveDirection direction, Material material, float amount);
            void displaceLiquidSideways(Grid& grid, int x, int y);
            
            void updatePowder(Grid& grid, int x, int y);
            void updateLiquid(Grid&, int x, int y, Material material);
            void updateWood(Grid& grid, int x, int y);
            
            void igniteWood(Grid& grid, int x, int y);
    };
}