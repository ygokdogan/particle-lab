#include <raylib.h>

#include "sandbox_lab/MaterialProperties.hpp"
#include "sandbox_lab/Renderer.hpp"


namespace sandbox {

    Renderer::Renderer(int cellSize) : cellSize(cellSize) {
    }
    
    void Renderer::render(const Grid& grid) {
        for (int y = 0; y < grid.getHeight(); ++y) {
            for (int x = 0; x < grid.getWidth(); ++x) {
                const Cell& currentCell = grid.getCell(x,y);
                
                if (currentCell.material == Material::Empty){
                    continue;
                }

                const MaterialProperties& props = getMaterialProperties(currentCell.material);
                Color color{props.color.r, props.color.g, props.color.b, 255};

                int screenX = x * cellSize;
                int screenY = y * cellSize;

                DrawRectangle(screenX, screenY, cellSize, cellSize, color);
            }
        }
    }
}