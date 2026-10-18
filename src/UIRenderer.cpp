#include "sandbox_lab/UIRenderer.hpp"
#include "sandbox_lab/MaterialProperties.hpp"

namespace sandbox
{
    struct MaterialEntry {
        Material material;
        const char* name;
        int key;
    };

    const MaterialEntry materials[] = {
        {Material::Stone, "Stone", 1},
        {Material::Sand,  "Sand",  2},
        {Material::Water, "Water", 3},
        {Material::Oil,   "Oil",   4}
    };

    void UIRenderer::load() {
        font = LoadFontEx("assets/fonts/Inter-Medium.ttf", 18, nullptr, 0);
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    }

    void UIRenderer::unload() {
        UnloadFont(font);
    }

    void UIRenderer::render(const Brush &brush, float simulationSpeed, bool paused) const {
        constexpr int panelWidth = 250;
        constexpr int panelHeight = 350;
        constexpr int padding = 16;

        int panelX = GetScreenWidth() - panelWidth - 12;
        int panelY = 12;

        int contentX = panelX + padding;

        DrawRectangle(
            panelX,
            panelY,
            panelWidth,
            panelHeight,
            Fade(BLACK, 0.85f)
        );

        drawHeader(contentX, panelY + 16);

        drawMaterials(
            contentX,
            panelY + 65,
            brush.getMaterial()
        );

        drawSimulationInfo(
            contentX,
            panelY + 200,
            brush.getSize(),
            simulationSpeed,
            paused
        );

        drawControls(contentX, panelY + 285);
    }

    void UIRenderer::drawText(const char* text, int x, int y, float size, Color color) const {
        DrawTextEx(font, text, Vector2{static_cast<float>(x), static_cast<float>(y)}, size, 0.5f, color);
    }

    void UIRenderer::drawHeader(int x, int y) const {
        drawText("SANDBOX LAB", x, y, 21, RAYWHITE);
        DrawLine(x, y + 33, x + 218, y + 33, DARKGRAY);
    }

    void UIRenderer::drawMaterials(int x, int y, Material selectedMaterial) const {
        drawText("MATERIALS", x, y, 13, GRAY);
        
        int rowY = y + 27;
        for (const MaterialEntry& entry : materials) {
            bool selected = entry.material == selectedMaterial;

            if (selected) {
                DrawRectangle(x-5, rowY -4, 226, 26, Fade(WHITE, 0.12f));
            }
            const auto& props = getMaterialProperties(entry.material);
            Color materialColor{props.color.r, props.color.g, props.color.b, 255};
            DrawRectangle(x, rowY + 3, 12, 12, materialColor);

            drawText(TextFormat("[%d] %s", entry.key, entry.name), x + 23, rowY, 15, selected ? WHITE : LIGHTGRAY);
            rowY += 27;
        }
    }

    void UIRenderer::drawSimulationInfo(int x, int y, int brushSize, float simulationSpeed, bool paused) const {
        drawText("Brush Size:", x, y, 14, LIGHTGRAY);

        drawText(TextFormat("%d", brushSize), x + 175, y, 14, WHITE);
        drawText("Sim Speed:", x, y + 27, 14, LIGHTGRAY);
        drawText(TextFormat("%.2fx", simulationSpeed), x + 160, y + 27, 14, WHITE);
        drawText("Status:", x, y + 54, 14, LIGHTGRAY);
        drawText(paused ? "PAUSED" : "RUNNING", x + 145, y + 54, 14, paused ? ORANGE : GREEN);
    }

    void UIRenderer::drawControls(int x, int y) const {
        DrawLine(x, y - 12, x + 218, y - 12, DARKGRAY);

        drawText("SPACE", x, y, 12, WHITE);
        drawText("Pause / Resume", x + 70, y, 12, GRAY);

        drawText("UP / DOWN", x, y + 20, 12, WHITE);
        drawText("Simulation Speed", x + 90, y + 20, 12, GRAY);

        drawText("SCROLL / +/-", x, y + 40, 12, WHITE);
        drawText("Brush Size", x + 105, y + 40, 12, GRAY);
    }
} // namespace sandbox