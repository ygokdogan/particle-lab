#include "sandbox_lab/UIRenderer.hpp"
#include "sandbox_lab/MaterialProperties.hpp"
#include <array>
#include <algorithm>
#include <cmath>

namespace sandbox {
    namespace {
        struct MaterialEntry {
            Material material;
            const char* name;
            int key;
        };

        constexpr std::array<MaterialEntry, 6> materials{{
            {Material::Stone, "Stone", 1},
            {Material::Wood, "Wood", 2},
            {Material::Sand, "Sand", 3},
            {Material::Water, "Water", 4},
            {Material::Oil, "Oil", 5},
            {Material::Lava, "Lava", 6}
        }};

        enum Icon { Pencil, Eraser, Play, Pause, Step, Minus, Plus, Trash };
        constexpr std::array<const char*, 8> iconPaths{{
            "assets/icons/pencil.png", "assets/icons/eraser.png",
            "assets/icons/play.png", "assets/icons/pause.png",
            "assets/icons/step-forward.png", "assets/icons/minus.png",
            "assets/icons/plus.png", "assets/icons/trash.png"
        }};

        constexpr std::array<int, 6> fontSizes{{10, 11, 12, 13, 14, 17}};

        constexpr Color background{23, 28, 35, 245};
        constexpr Color border{57, 68, 81, 255};
        constexpr Color accent{132, 177, 219, 255};
        constexpr Color textPrimary{232, 239, 246, 255};
        constexpr Color textSecondary{142, 156, 172, 255};

        Rectangle topBar() {
            return {86.0f, 12.0f, static_cast<float>(GetScreenWidth() - 98), 54.0f};
        }

        Rectangle bottomBar() {
            return {86.0f, static_cast<float>(GetScreenHeight() - 50), static_cast<float>(GetScreenWidth() - 98), 38.0f};
        }

        Rectangle materialBar() {
            return {16, 80, 64, 430};
        }

        Rectangle materialButton(int index) {
            return {24, 220.0f + index * 46.0f, 48, 40};
        }

        std::array<Rectangle, 5> controlButtons() {
            float x = static_cast<float>(GetScreenWidth() - 407);
            return {{
                {x, 22, 98, 34},
                {x + 106, 22, 54, 34},
                {x + 168, 22, 34, 34},
                {x + 272, 22, 34, 34},
                {x + 314, 22, 78, 34}
            }};
        }
    }

    void UIRenderer::load() {
        refreshFonts();
        for (std::size_t i = 0; i < icons.size(); ++i) {
            if (icons[i].id != 0) continue;
            Image image = LoadImage(iconPaths[i]);
            if (!image.data) continue;
            // Use the source alpha as a mask so dark icons can be tinted for the dark UI.
            ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
            auto* pixels = static_cast<Color*>(image.data);
            for (int pixel = 0; pixel < image.width * image.height; ++pixel) {
                pixels[pixel].r = pixels[pixel].g = pixels[pixel].b = 255;
            }
            icons[i] = LoadTextureFromImage(image);
            UnloadImage(image);
            if (icons[i].id != 0) SetTextureFilter(icons[i], TEXTURE_FILTER_BILINEAR);
        }
    }

    void UIRenderer::unload() {
        for (Texture2D& icon : icons) {
            if (icon.id != 0) UnloadTexture(icon);
            icon = {};
        }
        unloadFonts();
    }

    void UIRenderer::unloadFonts() {
        for (Font& font : fonts) {
            if (font.texture.id != 0 && font.texture.id != GetFontDefault().texture.id) UnloadFont(font);
            font = {};
        }
        fontScale = 0.0f;
    }

    void UIRenderer::refreshFonts() {
        Vector2 dpi = GetWindowScaleDPI();
        float scale = std::max(1.0f, std::max(dpi.x, dpi.y));
        if (fontScale == scale) return;

        unloadFonts();
        for (std::size_t i = 0; i < fonts.size(); ++i) {
            fonts[i] = LoadFontEx("assets/fonts/Inter-Medium.ttf",
                                  static_cast<int>(std::ceil(fontSizes[i] * scale)), nullptr, 0);
            if (fonts[i].texture.id != 0 && fonts[i].texture.id != GetFontDefault().texture.id) {
                SetTextureFilter(fonts[i].texture, TEXTURE_FILTER_BILINEAR);
            }
        }
        fontScale = scale;
    }

    const Font& UIRenderer::textFont(float size) const {
        std::size_t closest = 0;
        for (std::size_t i = 1; i < fontSizes.size(); ++i) {
            if (std::abs(fontSizes[i] - size) < std::abs(fontSizes[closest] - size)) closest = i;
        }
        return fonts[closest];
    }

    void UIRenderer::drawText(const char* text, float x, float y, float size, Color color) const {
        DrawTextEx(textFont(size), text, {x, y}, size, 0.5f, color);
    }

    void UIRenderer::drawIcon(int index, Rectangle bounds, Color color) const {
        const Texture2D& icon = icons[index];
        if (icon.id == 0) return;
        float scale = std::min(bounds.width / icon.width, bounds.height / icon.height);
        Rectangle destination{bounds.x + (bounds.width - icon.width * scale) * 0.5f,
                              bounds.y + (bounds.height - icon.height * scale) * 0.5f,
                              icon.width * scale, icon.height * scale};
        DrawTexturePro(icon, {0, 0, static_cast<float>(icon.width), static_cast<float>(icon.height)},
                       destination, {0, 0}, 0, color);
    }

    void UIRenderer::drawCenteredText(const char* text, Rectangle bounds, float size, Color color) const {
        Vector2 measured = MeasureTextEx(textFont(size), text, size, 0.5f);
        drawText(text, bounds.x + (bounds.width - measured.x) * 0.5f,
                 bounds.y + (bounds.height - measured.y) * 0.5f, size, color);
    }

    void UIRenderer::drawPanel(Rectangle bounds) const {
        DrawRectangleRounded(bounds, 0.18f, 8, background);
        DrawRectangleRoundedLinesEx(bounds, 0.18f, 8, 1.0f, border);
    }

    bool UIRenderer::isPointerOverUI() const {
        if (!visible) return false;
        Vector2 mouse = GetMousePosition();
        return CheckCollisionPointRec(mouse, topBar()) || CheckCollisionPointRec(mouse, materialBar()) || CheckCollisionPointRec(mouse, bottomBar());
    }

    UIActions UIRenderer::update(Brush& brush) {
        UIActions actions;
        refreshFonts();

        // Material shortcuts select the drawing tool just like material buttons.
        for (const MaterialEntry& entry : materials) {
            if (IsKeyPressed(KEY_ONE + entry.key - 1)) {
                brush.setMaterial(entry.material);
                eraserSelected = false;
            }
        }

        if (IsKeyPressed(KEY_TAB)) visible = !visible;
        if (!visible || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return actions;

        Vector2 mouse = GetMousePosition();

        if (CheckCollisionPointRec(mouse, Rectangle{24, 94, 48, 40})) {
            eraserSelected = false;
            return actions;
        }

        if (CheckCollisionPointRec(mouse, Rectangle{24, 142, 48, 40})) {
            eraserSelected = true;
            return actions;
        }

        for (int i = 0; i < static_cast<int>(materials.size()); ++i) {
            if (CheckCollisionPointRec(mouse, materialButton(i))) {
                brush.setMaterial(materials[i].material);
                eraserSelected = false;
                return actions;
            }
        }

        auto buttons = controlButtons();
        if (CheckCollisionPointRec(mouse, buttons[0])) actions.togglePause = true;
        if (CheckCollisionPointRec(mouse, buttons[1])) actions.stepSimulate = true;
        if (CheckCollisionPointRec(mouse, buttons[2])) actions.speedDown = true;
        if (CheckCollisionPointRec(mouse, buttons[3])) actions.speedUp = true;
        if (CheckCollisionPointRec(mouse, buttons[4])) actions.clear = true;

        return actions;
    }

    void UIRenderer::render(const Brush& brush, float simulationSpeed, bool paused) const {
        if (!visible) return;

        Vector2 mouse = GetMousePosition();
        drawPanel(topBar());
        drawPanel(materialBar());
        drawPanel(bottomBar());

        Rectangle header = topBar();
        drawText("PARTICLE LAB", 103, header.y + (header.height - 17) * 0.5f, 17, textPrimary);
        drawText("CELLULAR PHYSICS", 248, header.y + (header.height - 11) * 0.5f, 11, textSecondary);

        for (int i = 0; i < static_cast<int>(materials.size()); ++i) {
            const MaterialEntry& entry = materials[i];
            Rectangle bounds = materialButton(i);
            bool selected = !eraserSelected && brush.getMaterial() == entry.material;
            bool hovered = CheckCollisionPointRec(mouse, bounds);

            Color buttonColor = selected ? Color{49, 73, 96, 255} : hovered ? Color{45, 53, 63, 255} : Color{32, 39, 48, 255};
            DrawRectangleRounded(bounds, 0.22f, 8, buttonColor);
            if (selected) DrawRectangleRoundedLinesEx(bounds, 0.22f, 8, 1.5f, accent);

            const auto& c = getMaterialProperties(entry.material).color;
            Color color{c.r, c.g, c.b, 255};
            DrawRectangleRounded({bounds.x + 13, bounds.y + 10, 22, 22}, 0.22f, 6, color);
            drawText(TextFormat("%d", entry.key), bounds.x + 37, bounds.y + 3, 10, textSecondary);

            if (hovered) {
                const char* label = TextFormat("%s [%d]", entry.name, entry.key);
                float width = MeasureTextEx(textFont(14), label, 14, 0.5f).x + 24;
                Rectangle tooltip{80, bounds.y + 3, width, 34};
                drawPanel(tooltip);
                drawCenteredText(label, tooltip, 14, textPrimary);
            }
        }

        auto buttons = controlButtons();
        const char* labels[] = {paused ? "Resume" : "Pause", "Step", "-", "+", "Clear"};
        const char* tooltips[] = {paused ? "Resume [Space]" : "Pause [Space]", "Step [N]",
                                  "Slower [Down]", "Faster [Up]", "Clear"};
        const int buttonIcons[] = {paused ? Play : Pause, Step, Minus, Plus, Trash};

        for (int i = 0; i < 5; ++i) {
            Rectangle bounds = buttons[i];
            bool hovered = CheckCollisionPointRec(mouse, bounds);
            Color color = hovered ? Color{59, 76, 95, 255} : Color{38, 47, 58, 255};
            if (i == 0 && paused) color = {49, 73, 96, 255};

            DrawRectangleRounded(bounds, 0.22f, 8, color);
            Color tint = i == 4 ? Color{235, 151, 151, 255} : textPrimary;
            if (icons[buttonIcons[i]].id != 0) {
                drawIcon(buttonIcons[i], {bounds.x + (bounds.width - 20) * 0.5f, bounds.y + 7, 20, 20}, tint);
            } else {
                drawCenteredText(labels[i], bounds, 13, tint);
            }
            if (hovered) {
                float width = std::max(bounds.width, MeasureTextEx(textFont(13), tooltips[i], 13, 0.5f).x + 16);
                float x = std::max(8.0f, std::min(bounds.x, GetScreenWidth() - width - 8));
                Rectangle tooltip{x, bounds.y + bounds.height + 6, width, 30};
                drawPanel(tooltip);
                drawCenteredText(tooltips[i], tooltip, 13, textPrimary);
            }
        }

        float speedLeft = buttons[2].x + buttons[2].width;
        Rectangle speedBounds{speedLeft, buttons[2].y, buttons[3].x - speedLeft, buttons[2].height};
        drawCenteredText(TextFormat("%.2fx", simulationSpeed), speedBounds, 13, textPrimary);

        Rectangle pen{24, 94, 48, 40};
        Rectangle eraser{24, 142, 48, 40};

        const Rectangle toolButtons[] = {pen, eraser};
        const int toolIcons[] = {Pencil, Eraser};
        const char* toolLabels[] = {"Brush [Left Mouse]", "Eraser [Right Mouse]"};
        for (int i = 0; i < 2; ++i) {
            Rectangle bounds = toolButtons[i];
            bool selected = i == 0 ? !eraserSelected : eraserSelected;
            bool hovered = CheckCollisionPointRec(mouse, bounds);
            Color backgroundColor = hovered ? Color{59, 76, 95, 255}
                                            : selected ? Color{49, 73, 96, 255} : Color{39, 47, 57, 255};
            DrawRectangleRounded(bounds, 0.2f, 8, backgroundColor);
            if (selected) DrawRectangleRoundedLinesEx(bounds, 0.2f, 8, 1.5f, accent);
            drawIcon(toolIcons[i], {bounds.x + 12, bounds.y + 8, 24, 24},
                     selected ? Color{173, 212, 250, 255} : LIGHTGRAY);

            if (hovered) {
                float width = MeasureTextEx(textFont(14), toolLabels[i], 14, 0.5f).x + 24;
                Rectangle tooltip{80, bounds.y + 3, width, 34};
                drawPanel(tooltip);
                drawCenteredText(toolLabels[i], tooltip, 14, textPrimary);
            }
        }

        const char* materialName = "Unknown";
        for (const MaterialEntry& entry : materials) {
            if (entry.material == brush.getMaterial()) materialName = entry.name;
        }

        const auto& c = getMaterialProperties(brush.getMaterial()).color;
        Color selectedColor{c.r, c.g, c.b, 255};
        Rectangle footer = bottomBar();
        float statusCenterY = footer.y + footer.height * 0.5f;
        float statusY = statusCenterY - 13 * 0.5f;

        DrawCircleV({106, statusCenterY}, 5, selectedColor);
        drawText(eraserSelected ? "Eraser" : materialName, 120, statusY, 13, textPrimary);
        drawText(TextFormat("Brush: %d", brush.getSize()), 252, statusY, 13, textSecondary);
        drawText(paused ? "PAUSED" : "RUNNING", 380, statusCenterY - 6, 12, paused ? Color{238, 178, 110, 255} : Color{118, 205, 155, 255});
        drawText("TAB  Hide UI", GetScreenWidth() - 285, statusCenterY - 6, 12, textSecondary);
        drawText(TextFormat("%d FPS", GetFPS()), GetScreenWidth() - 95, statusCenterY - 6, 12, textPrimary);
    }
}
