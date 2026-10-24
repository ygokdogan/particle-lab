#include "sandbox_lab/Brush.hpp"

namespace sandbox {
    Brush::Brush(int size, Material material) : size(size), selectedMaterial(material) {
        setSize(size);
        setMaterial(material);
    }

    void Brush::paint(Grid &grid, int x, int y) const {
        Behaviour behaviour = getMaterialProperties(selectedMaterial).behaviour;

        int radius = size / 2;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (behaviour == Behaviour::Liquid) {
                    grid.addLiquid(x + dx, y + dy, selectedMaterial, 1.0f);
                } else {
                    grid.placeSolid(x + dx, y + dy, selectedMaterial);
                }
            }
        }
    }

    void Brush::erase(Grid &grid, int x, int y) const {
        int radius = size / 2;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                grid.eraseCell(x + dx, y + dy);
            }
        }
    }

    void Brush::setSize(int newSize) {
        if (newSize <= 0) {
            newSize = 1;
        }

        if (newSize % 2 == 0) {
            newSize -= 1;
        }

        size = newSize;
    }

    int Brush::getSize() const {
        return size;
    }

    void Brush::setMaterial(Material newMaterial) {
        selectedMaterial = newMaterial;
    }

    Material Brush::getMaterial() const {
        return selectedMaterial;
    }
} // namespace sandbox
