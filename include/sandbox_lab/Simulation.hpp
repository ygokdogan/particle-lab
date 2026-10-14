#pragma once

#include "Grid.hpp"

namespace sandbox {
    class Simulation {
        public:
            void update(Grid& grid);

        private: 
            void updatePowder(Grid& grid, int x, int y);
    };
}