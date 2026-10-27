#include "sandbox_lab/Application.hpp"
#include "sandbox_lab/UILayout.hpp"
#include "raylib.h"
#include "rlgl.h"


#include <algorithm>
#include <cassert>
#include <cmath>

namespace sandbox {
   Application::Application()
        : grid(UILayout::canvasWidth / UILayout::cellSize, UILayout::canvasHeight / UILayout::cellSize),
        simulation(), renderer(UILayout::cellSize), brush(3), inputHandler(UILayout::cellSize), uiRenderer() {
        SetConfigFlags(FLAG_WINDOW_HIGHDPI);
        InitWindow(UILayout::windowWidth, UILayout::windowHeight, "Particle Lab");
        uiRenderer.load();
        SetTargetFPS(60);
    }

    Application::~Application() {
        uiRenderer.unload();
        CloseWindow();
    }

    void Application::run() {
        while (!WindowShouldClose()) {
            UIActions uiActions = uiRenderer.update(brush);
            InputActions actions = inputHandler.update(grid, brush, UILayout::canvasX, UILayout::canvasY, uiRenderer.isEraserSelected(), uiRenderer.isPointerOverUI());

            actions.togglePause |= uiActions.togglePause;
            actions.speedUp |= uiActions.speedUp;
            actions.speedDown |= uiActions.speedDown;
            actions.stepSimulate |= uiActions.stepSimulate;

            if (uiActions.clear) {
                for (int y = 0; y < grid.getHeight(); ++y) {
                    for (int x = 0; x < grid.getWidth(); ++x) grid.eraseCell(x, y);
                }
                simulationAccumulator = 0.0f;
            }

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
            } else {
                if (actions.stepSimulate) {
                    simulation.update(grid);
                }
            }

            BeginDrawing();
            ClearBackground(Color{20, 25, 32, 255});

            Rectangle frame{UILayout::canvasX - 6.0f, UILayout::canvasY - 6.0f, UILayout::canvasWidth + 12.0f, UILayout::canvasHeight + 12.0f};
            DrawRectangleRounded(frame, 0.02f, 8, Color{38, 47, 58, 255});
            DrawRectangleRoundedLinesEx(frame, 0.02f, 8, 1.0f, Color{65, 79, 95, 255});

            DrawRectangle(UILayout::canvasX, UILayout::canvasY, UILayout::canvasWidth, UILayout::canvasHeight, Color{9, 14, 20, 255});

            BeginScissorMode(UILayout::canvasX, UILayout::canvasY, UILayout::canvasWidth, UILayout::canvasHeight);
            rlPushMatrix();
            rlTranslatef(static_cast<float>(UILayout::canvasX), static_cast<float>(UILayout::canvasY), 0.0f);

            renderer.render(grid);

            auto hoveredCell = inputHandler.getHoveredCell();
            if (hoveredCell) {
                auto [x, y] = *hoveredCell;
                renderer.renderBrushPreview(brush, x, y, uiRenderer.isEraserSelected() || IsMouseButtonDown(MOUSE_BUTTON_RIGHT));
            }

            rlPopMatrix();
            EndScissorMode();

            uiRenderer.render(brush, simulationSpeed, paused);
            EndDrawing();
        }
    }
} // namespace sandbox