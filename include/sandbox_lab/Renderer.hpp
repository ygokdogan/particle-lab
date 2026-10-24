#pragma once

#include "Brush.hpp"
#include "Grid.hpp"

namespace sandbox {
    class Renderer {
        public:
            explicit Renderer(int cellSize);
            void render(const Grid& grid);
            void renderBrushPreview(const Brush& brush, int gridX, int gridY) const;

        private:
            int cellSize;
            
            void renderSolidCell(const Cell& cell, int x, int y) const;
            void renderLiquidCell(const Cell& cell, int x, int y) const;
            void renderSmokeCell(const Cell& cell, int x, int y) const;
    };
}