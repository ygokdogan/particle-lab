#pragma once

#include "Grid.hpp"
#include <random>

namespace sandbox {
    class Simulation {
    public:
        void update(Grid &grid);

    private:
        std::mt19937 rng;
        std::uniform_int_distribution<int> randomDirection{0, 1};

        std::uint64_t currentFrame;
        bool scanLeftToRight = true;
    };
} // namespace sandbox