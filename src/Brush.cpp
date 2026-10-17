#include "sandbox_lab/Brush.hpp"

namespace sandbox
{
    Brush::Brush(int size, Material material) : size(size), selectedMaterial(material)
    {
        setSize(size);
        setMaterial(material);
    }

    void Brush::paint(Grid& grid, int x, int y) {
        if(!grid.isValidCell(x, y)){
            return;
        }

        Behaviour behaviour = getMaterialProperties(selectedMaterial).behaviour;
        
        for(int i = y-(size/2); i <= y+(size/2); i++) {
            for (int j = x-(size/2); j <= x+(size/2); j++) {
                if(behaviour == Behaviour::Liquid) {
                    grid.addLiquid(j, i, selectedMaterial, 1.0f);
                } else {
                    grid.placeSolid(j, i, selectedMaterial);
                }
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
