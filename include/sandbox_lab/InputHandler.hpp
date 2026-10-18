#pragma once

#include <optional>
#include <utility>

#include "Brush.hpp"
#include "Grid.hpp"
#include "Material.hpp"

namespace sandbox
{
    struct InputActions {
        bool togglePause = false;
        bool speedUp = false;
        bool speedDown = false;
        bool stepSimulate = false;
    };

    class InputHandler {
        public:
            explicit InputHandler(int cellSize);

            InputActions update(Grid& grid, Brush& brush);

            std::optional<std::pair<int, int>> getHoveredCell() const;
        private: 
            int cellSize;
            std::optional<std::pair<int,int>> previousMouseCell;
            std::optional<std::pair<int,int>> hoveredCell;
            bool previousErasing = false;

            InputActions handleSimulationControls();

            void drawLine(Grid& grid, Brush& brush, int x0, int y0, int x1, int y1, bool erasing);

            void handleMaterialSelection(Brush& brush);
            void handleBrushSizeChange(Brush& brush);
    };
} // namespace sandbox
