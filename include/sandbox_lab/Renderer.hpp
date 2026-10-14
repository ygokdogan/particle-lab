#pragma once

#include "Grid.hpp"

namespace sandbox {
    class Renderer {
        public:
            explicit Renderer(int cellSize);
            void render(const Grid& grid);

        private:
            int cellSize;
    };
}