#include "sandbox_lab/InputHandler.hpp"
#include "raylib.h"

namespace sandbox
{
    InputHandler::InputHandler(int cellSize) : cellSize(cellSize)
    {
        
    }

    void InputHandler::update(Grid& grid, Brush& brush) {
        handleBrushSizeChange(brush);
        handleMaterialSelection(brush);
        handlePlacing(grid, brush);
    }

    void InputHandler::handlePlacing(Grid& grid, Brush& brush) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();

            int x = mousePos.x / cellSize;
            int y = mousePos.y / cellSize;

            brush.paint(grid, x, y);
        }
    }

    void InputHandler::handleMaterialSelection(Brush& brush) {
        if (IsKeyPressed(KEY_ONE)) {
            brush.setMaterial(Material::Sand);
        }
        if (IsKeyPressed(KEY_TWO)) {
            brush.setMaterial(Material::Water);
        }
    }

    void InputHandler::handleBrushSizeChange(Brush& brush) {
        if (IsKeyPressed(KEY_EQUAL)) {
            brush.setSize(brush.getSize()+2);
        }
        if (IsKeyPressed(KEY_MINUS)) {
            brush.setSize(brush.getSize()-2);
        }
    }
 } // namespace sandbox