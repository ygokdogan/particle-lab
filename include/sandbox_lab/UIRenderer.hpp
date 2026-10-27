#pragma once

#include <raylib.h>
#include <array>
#include "Brush.hpp"

namespace sandbox {
    struct UIActions {
        bool togglePause = false;
        bool stepSimulate = false;
        bool speedUp = false;
        bool speedDown = false;
        bool clear = false;
    };

    class UIRenderer {
        public:
            void load();
            void unload();
            UIActions update(Brush &brush);

            bool isPointerOverUI() const;
            bool isEraserSelected() const { return eraserSelected; }

            void render(const Brush &brush, float simulationSpeed, bool paused) const;
        private:
            std::array<Font, 6> fonts{};
            std::array<Texture2D, 8> icons{};
            float fontScale = 0.0f;
            void refreshFonts();
            void unloadFonts();
            const Font& textFont(float size) const;
            void drawIcon(int index, Rectangle bounds, Color color) const;
            void drawButton(Rectangle bounds, bool selected, Color tint) const;

            bool visible = true;
            bool eraserSelected = false;

            void drawPanel(Rectangle bounds) const;
            void drawCenteredText(const char* text, Rectangle bounds, float size, Color color) const;
            void drawText(const char *text, float x, float y, float size, Color color) const;
    };
} // namespace sandbox
