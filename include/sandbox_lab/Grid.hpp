#pragma once

#include <vector>
#include "Cell.hpp"

namespace sandbox
{
    class Grid {
        public: 
            Grid(int width, int height);

            Cell& getCell(int x, int y);
            const Cell& getCell(int x, int y) const;
            int getWidth() const;
            int getHeight() const;

            bool isValidCell(int x, int y) const;
            bool isEmptyCell(int x, int y) const;
            
            bool placeSolid(int x, int y, Material material);

            float getTotalLiquidAmount(int x, int y) const;
            float getLiquidAmount(int x, int y, Material material) const;
            float addLiquid(int x, int y, Material material, float amount);
            float removeLiquid(int x, int y, Material material, float amount);

        private:
            int width;
            int height;
            std::vector<Cell> cells;

            LiquidLayer* getLiquidLayer(int x, int y, Material material);

    };
}