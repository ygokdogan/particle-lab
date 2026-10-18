#include "sandbox_lab/Application.hpp"
#include "raylib.h"

#include <cassert>
#include <cmath>

namespace sandbox {
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 960;
    constexpr int cellSize = 8;


    Application::Application() : 
        grid(windowWidth / cellSize, windowHeight / cellSize),
        simulation(),
        renderer(cellSize),
        brush(3),
        inputHandler(cellSize),
        uiRenderer()
    {
        InitWindow(windowWidth, windowHeight, "Sandbox Lab");
        uiRenderer.load();
        SetTargetFPS(60);
    }

    Application::~Application() {
        uiRenderer.unload();
        CloseWindow();
    }

    void Application::run() {
        while (!WindowShouldClose()) {
            InputActions actions = inputHandler.update(grid, brush);

            if (actions.togglePause) {
                paused = !paused;
            }

            if (actions.speedUp) {
                simulationSpeed = std::min(simulationSpeed * 2.0f, 4.0f);
            }

            if (actions.speedDown) {
                simulationSpeed = std::max(simulationSpeed / 2.0f, 0.25f);
            }

            if (!paused) {
                simulationAccumulator += simulationSpeed;

                while (simulationAccumulator >= 1.0f) {
                    simulation.update(grid);
                    simulationAccumulator -= 1.0f;
                }
            }
            else {
                if(actions.stepSimulate) {
                    simulation.update(grid);
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);
            renderer.render(grid);
            auto hoveredCell = inputHandler.getHoveredCell();
            if(hoveredCell) {
                auto [x, y] = *hoveredCell;
                renderer.renderBrushPreview(brush, x, y);
            }

            uiRenderer.render(brush, simulationSpeed, paused);
            EndDrawing();
        }
    }
}