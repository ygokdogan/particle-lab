#pragma once

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

            void handlePlacing(Grid& grid, Brush& brush);
            void handleMaterialSelection(Brush& brush);
            void handleBrushSizeChange(Brush& brush);
    };
} // namespace sandbox
