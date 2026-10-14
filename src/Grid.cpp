#include <stdexcept>
#include "sandbox_lab/Grid.hpp"

namespace sandbox
{
    Grid::Grid(int width, int height) 
        : width(width), height(height), cells(width * height) {
    }

    Cell& Grid::getCell(int x, int y) {
        if (!isValidCell(x, y)) {
            throw std::out_of_range("Cell coordinates are out of bounds");
        }
        
        int index = y * width + x;
        return cells[index];
    }

    const Cell& Grid::getCell(int x, int y) const {
        if (!isValidCell(x, y)) {
            throw std::out_of_range("Cell coordinates are out of bounds");
        }

        int index = y * width + x;
        return cells[index];
    }

    int Grid::getWidth() const {
        return width;
    }

    int Grid::getHeight() const {
        return height;
    }

    bool Grid::isValidCell(int x, int y) const {
        return (x >= 0 && x < width && y >= 0 && y < height);
    }

    bool Grid::isEmptyCell(int x, int y) const {
        if (!isValidCell(x, y)) {
            return false;
        }
        
        return getCell(x, y).material == Material::Empty;
    }
}