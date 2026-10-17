#pragma once

#include "Grid.hpp"

namespace sandbox {
    class Renderer {
        public:
            explicit Renderer(int cellSize);
            void render(const Grid& grid);

        private:
            int cellSize;
            
            void renderSolidCell(const Cell& cell, int x, int y) const;
            void renderLiquidCell(const Cell& cell, int x, int y) const;
    };
}