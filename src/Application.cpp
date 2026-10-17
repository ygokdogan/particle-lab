#include "sandbox_lab/Application.hpp"
#include "raylib.h"

#include <cassert>
#include <cmath>

namespace sandbox {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;
    constexpr int cellSize = 8;


    Application::Application() : 
        grid(windowWidth / cellSize, windowHeight / cellSize),
        simulation(),
        renderer(cellSize),
        brush(5),
        inputHandler(cellSize)
    {
        InitWindow(windowWidth, windowHeight, "Sandbox Lab");
        SetTargetFPS(60);
    }

    Application::~Application() {
        CloseWindow();
    }

    void Application::run() {
        while (!WindowShouldClose()) {
            inputHandler.update(grid, brush);
            simulation.update(grid);

            BeginDrawing();
            ClearBackground(BLACK);
            renderer.render(grid);
            EndDrawing();
        }
    }

    float Application::getTotalMaterialAmount(const Grid& grid, Material material) {
        double total = 0.0f;

        for (int y = 0; y < grid.getHeight(); ++y) {
            for (int x = 0; x < grid.getWidth(); ++x) {
                total += grid.getLiquidAmount(x, y, material);
            }
        }

        return total;
    }
}