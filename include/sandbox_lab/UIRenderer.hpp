#pragma once

#include <raylib.h>
#include "Brush.hpp"

namespace sandbox {
    class UIRenderer {
    public:
        void load();
        void unload();

        void render(const Brush &brush, float simulationSpeed, bool paused) const;

    private:
        Font font{};

        void drawHeader(int x, int y) const;
        void drawMaterials(int x, int y, Material selectedMaterial) const;
        void drawSimulationInfo(int x, int y, int brushSize, float simulationSpeed, bool paused) const;
        void drawControls(int x, int y) const;

        void drawText(const char *text, int x, int y, float size, Color color) const;
    };
} // namespace sandbox
