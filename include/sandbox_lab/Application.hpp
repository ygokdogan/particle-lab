#pragma once

#include "Brush.hpp"
#include "Grid.hpp"
#include "InputHandler.hpp"
#include "Renderer.hpp"
#include "Simulation.hpp"
#include "UIRenderer.hpp"

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
        UIRenderer uiRenderer;

        bool paused = false;
        float simulationSpeed = 1.0f;
        float simulationAccumulator = 0.0f;
    };
}
