#include "sandbox_lab/InputHandler.hpp"
#include "raylib.h"

#include <cstdlib>

namespace sandbox
{
    InputHandler::InputHandler(int cellSize) : cellSize(cellSize)
    {
        
    }

    InputActions InputHandler::handleSimulationControls() {
        InputActions actions;

        actions.togglePause = IsKeyPressed(KEY_SPACE);
        actions.speedUp = IsKeyPressed(KEY_UP);
        actions.speedDown = IsKeyPressed(KEY_DOWN);

        return actions;
    }

    InputActions InputHandler::update(Grid& grid, Brush& brush) {
        InputActions actions = handleSimulationControls();
        handleBrushSizeChange(brush);
        handleMaterialSelection(brush);
        
        bool erasing = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
        bool painting = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        int mouseX = GetMouseX();
        int mouseY = GetMouseY();

        int gridX = mouseX / cellSize;
        int gridY = mouseY / cellSize;

        if (mouseX < 0 || mouseY < 0 ||
            !grid.isValidCell(gridX, gridY)) {
            previousMouseCell.reset();
            hoveredCell.reset();
            return actions;
        }

        hoveredCell = std::make_pair(gridX, gridY);
        
        if(!erasing && !painting){
            previousMouseCell.reset();
            return actions;
        }

        if (!previousMouseCell ||
            previousErasing != erasing) {
            previousMouseCell = {gridX, gridY};
        }

        auto [previousX, previousY] = *previousMouseCell;

        drawLine(grid, brush, previousX, previousY, gridX, gridY, erasing);

        previousMouseCell = {gridX, gridY};
        previousErasing = erasing;

        return actions;
    }

    void InputHandler::drawLine(Grid& grid, Brush& brush, int x0, int y0, int x1, int y1, bool erasing) {
        int dx = std::abs(x1-x0);
        int dy = std::abs(y1-y0);

        int stepX = x0 < x1 ? 1 : -1;
        int stepY = y0 < y1 ? 1 : -1;

        int error = dx - dy;

        while (true) {
            if (erasing) {
                brush.erase(grid, x0, y0);
            }
            else {
                brush.paint(grid, x0, y0);
            }

            if (x0 == x1 && y0 == y1) {
                break;
            }

            int doubleError = 2 * error;
            if(doubleError > -dy) {
                error -= dy;
                x0 += stepX;
            }

            if (doubleError < dx) {
                error += dx;
                y0 += stepY;
            }
        }
    }

    void InputHandler::handleMaterialSelection(Brush& brush) {
        if (IsKeyPressed(KEY_ONE)) {
            brush.setMaterial(Material::Stone);
        }
        if (IsKeyPressed(KEY_TWO)) {
            brush.setMaterial(Material::Sand);
        }
        if (IsKeyPressed(KEY_THREE)) {
            brush.setMaterial(Material::Water);
        }
        if (IsKeyPressed(KEY_FOUR)) {
            brush.setMaterial(Material::Oil);
        }
    }

    void InputHandler::handleBrushSizeChange(Brush& brush) {
        if (IsKeyPressed(KEY_EQUAL)) {
            brush.setSize(brush.getSize()+2);
        }
        if (IsKeyPressed(KEY_MINUS)) {
            brush.setSize(brush.getSize()-2);
        }

        float wheel = GetMouseWheelMove();
        if (wheel > 0.0f) {
            brush.setSize(brush.getSize() + 2);
        }
        else if (wheel < 0.0f) {
            brush.setSize(brush.getSize() - 2);
        }
    }

    std::optional<std::pair<int,int>> InputHandler::getHoveredCell() const {
        return hoveredCell;
    }
 } // namespace sandbox