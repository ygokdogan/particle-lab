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
                
                int screenX = x * cellSize;
                int screenY = y * cellSize;
                
                if (currentCell.solid != Material::Empty){
                    const MaterialProperties& props = getMaterialProperties(currentCell.solid);
                    ::Color color{props.color.r, props.color.g, props.color.b, 255};
                    DrawRectangle(screenX, screenY, cellSize, cellSize, color);
                    continue;
                }

                if (currentCell.liquids.empty()) {
                    continue;
                }

                const LiquidLayer* dominantLayer = &currentCell.liquids[0];

                for (const LiquidLayer& layer : currentCell.liquids) {
                    if (layer.amount > dominantLayer->amount) {
                        dominantLayer = &layer;
                    }
                }

                const MaterialProperties& props =
                    getMaterialProperties(dominantLayer->material);

                ::Color color{
                    props.color.r,
                    props.color.g,
                    props.color.b,
                    255
                };

                DrawRectangle(screenX, screenY, cellSize, cellSize, color);
            }
        }
    }
}