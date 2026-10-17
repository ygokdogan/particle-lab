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

            float waterBefore = getTotalMaterialAmount(grid, Material::Water);
            float oilBefore = getTotalMaterialAmount(grid, Material::Oil);

            simulation.update(grid);

            float waterAfter = getTotalMaterialAmount(grid, Material::Water);
            float oilAfter = getTotalMaterialAmount(grid, Material::Oil);

            double waterDiff = waterAfter - waterBefore;
            double oilDiff = oilAfter - oilBefore;

            if (std::fabs(waterDiff) >= 0.001) {
                std::fprintf(
                    stderr,
                    "WATER | Before: %.9f | After: %.9f | Diff: %.9f\n",
                    waterBefore,
                    waterAfter,
                    waterDiff
                );
            }

            if (std::fabs(oilDiff) >= 0.001) {
                std::fprintf(
                    stderr,
                    "OIL | Before: %.9f | After: %.9f | Diff: %.9f\n",
                    oilBefore,
                    oilAfter,
                    oilDiff
                );
            }

            double waterTolerance = std::max(0.001, waterBefore * 0.00001);
            double oilTolerance = std::max(0.001, oilBefore * 0.00001);

            assert(std::fabs(waterBefore - waterAfter) < waterTolerance);
            assert(std::fabs(oilBefore - oilAfter) < oilTolerance);

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