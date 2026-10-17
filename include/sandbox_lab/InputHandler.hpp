#pragma once

#include <optional>
#include <utility>

#include "Brush.hpp"
#include "Grid.hpp"
#include "Material.hpp"

namespace sandbox
{
    class InputHandler {
        public:
            explicit InputHandler(int cellSize);

            void update(Grid& grid, Brush& brush);

        private: 
            int cellSize;
            std::optional<std::pair<int,int>> previousMouseCell;
            bool previousErasing = false;

            void drawLine(Grid& grid, Brush& brush, int x0, int y0, int x1, int y1, bool erasing);

            void handlePlace(Grid& grid, Brush& brush);
            void handleErase(Grid& grid, Brush& brush);
            void handleMaterialSelection(Brush& brush);
            void handleBrushSizeChange(Brush& brush);
    };
} // namespace sandbox
