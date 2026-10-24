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

                if (currentCell.solid != Material::Empty) {
                    renderSolidCell(currentCell, x, y);
                }
                else if (!currentCell.liquids.empty()) {
                    renderLiquidCell(currentCell, x, y);
                }
                else if (currentCell.gas == Material::Smoke) {
                    renderSmokeCell(currentCell, x, y);
                }
            }
        }
    }

    void Renderer::renderSolidCell(const Cell& cell, int x, int y) const {
        const MaterialProperties& props = getMaterialProperties(cell.solid);

        ::Color color {props.color.r, props.color.g, props.color.b, 255};

        if (cell.solid == Material::Wood && cell.burnTicks > 0) {
            float progress = static_cast<float>(cell.burnTicks) / cell.burnDuration;

            ::Color burningColor{255, 115, 35, 255};
            ::Color charredColor{45, 35, 30, 255};

            color = ColorLerp(color, burningColor, 0.7f);
            color = ColorLerp(color, charredColor, progress);
        }

        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);
    }

    void Renderer::renderLiquidCell(const Cell& cell, int x, int y) const {
        constexpr float renderThreshold = 0.25f;

        const LiquidLayer* dominantLayer = nullptr;
        for (const LiquidLayer& layer : cell.liquids) {
            if (dominantLayer == nullptr || layer.amount > dominantLayer->amount) {
                dominantLayer = &layer;
            }
        }

        if(dominantLayer == nullptr || dominantLayer->amount < renderThreshold) {
            return;
        }

        const MaterialProperties& props = getMaterialProperties(dominantLayer->material);
        ::Color color{props.color.r, props.color.g, props.color.b, 255};
        DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);
    }

    void Renderer::renderBrushPreview(const Brush &brush, int gridX, int gridY) const {
        int brushSize = brush.getSize();
        int radius = brushSize / 2;

        int screenX = (gridX - radius) * cellSize;
        int screenY = (gridY - radius) * cellSize;

        int previewSize = brushSize * cellSize;

        const MaterialProperties& props = getMaterialProperties(brush.getMaterial());

        ::Color color{
            props.color.r,
            props.color.g,
            props.color.b,
            255
        };

        Rectangle bounds{
            static_cast<float>(screenX),
            static_cast<float>(screenY),
            static_cast<float>(previewSize),
            static_cast<float>(previewSize)
        };

        DrawRectangleRec(bounds, Fade(color, 0.18f));
        DrawRectangleLinesEx(bounds, 1.0f, Fade(color, 0.8f));
    }

    void Renderer::renderSmokeCell(const Cell& cell, int x, int y) const {
        const MaterialProperties& props = getMaterialProperties(cell.gas);

        float opacity = std::clamp(static_cast<float>(cell.gasLifetime) / 150.0f, 0.0f, 1.0f);
        ::Color color{props.color.r, props.color.g, props.color.b, 255};

        DrawRectangle(
            x * cellSize,
            y * cellSize,
            cellSize,
            cellSize,
            Fade(color, opacity * 0.65f)
        );
    }
}