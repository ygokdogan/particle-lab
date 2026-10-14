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

        private:
            int width;
            int height;
            std::vector<Cell> cells;
    };
}