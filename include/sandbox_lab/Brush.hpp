#pragma once

#include "Grid.hpp"
#include "Material.hpp"
#include "MaterialProperties.hpp"

namespace sandbox {
    class Brush {
        public: 
            explicit Brush(int size, Material material = Material::Sand);
            void paint(Grid& grid, int x, int y) const;
            void erase(Grid& grid, int x, int y) const;

            void setSize(int newSize);
            int getSize() const;

            void setMaterial(Material newMaterial);
            Material getMaterial() const;
        private:
            int size;
            Material selectedMaterial;
    };
}