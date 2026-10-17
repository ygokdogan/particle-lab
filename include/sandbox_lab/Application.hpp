#pragma once

#include "Brush.hpp"
#include "Grid.hpp"
#include "InputHandler.hpp"
#include "Renderer.hpp"
#include "Simulation.hpp"

namespace sandbox {
    class Application {
        public:
            Application();
            ~Application();

            void run();
        private:
            Grid grid;
            Simulation simulation;
            Renderer renderer;
            Brush brush;
            InputHandler inputHandler;

            float getTotalMaterialAmount(const Grid& grid, Material material);
    };
}
