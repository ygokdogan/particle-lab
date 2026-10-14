#include <iostream>
#include <cassert>
#include "raylib.h"

#include "sandbox_lab/Cell.hpp"
#include "sandbox_lab/Grid.hpp"
#include "sandbox_lab/MaterialProperties.hpp"
#include "sandbox_lab/Renderer.hpp"
#include "sandbox_lab/Simulation.hpp"

int main() {
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;
    constexpr int cellSize = 10;

    InitWindow(windowWidth, windowHeight, "Sandbox Lab");

    SetTargetFPS(60);

    sandbox::Grid grid(windowWidth / cellSize, windowHeight / cellSize);
    sandbox::Simulation simulation;
    sandbox::Renderer renderer(cellSize);

    grid.getCell(2,1).material = sandbox::Material::Sand;

    while (!WindowShouldClose()) {
        simulation.update(grid);
        BeginDrawing();
        ClearBackground(BLACK);
        renderer.render(grid);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}