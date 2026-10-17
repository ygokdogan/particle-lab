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

            const int maxFlowDistance = 12;

            bool moveCell(Grid& grid, int x, int y, MoveDirection direction);
            bool canFlowTowardsLowerLevel(const Grid& grid, int x, int y, MoveDirection direction);
            
            void updatePowder(Grid& grid, int x, int y);
            void updateLiquid(Grid&, int x, int y);
    };
}