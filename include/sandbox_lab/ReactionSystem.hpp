#pragma once

#include "Grid.hpp"
#include <random>

namespace sandbox {
    class GasPhysics;

    class ReactionSystem {
    public:
        ReactionSystem(std::mt19937 &rng, GasPhysics &gasPhysics);

        void updateWood(Grid &grid, int x, int y);
        void updateLava(Grid &grid, int x, int y);
        void coolLava(Grid &grid);

    private:
        std::mt19937 &rng;
        GasPhysics &gasPhysics;

        void igniteWood(Grid &grid, int x, int y);
    };
}
