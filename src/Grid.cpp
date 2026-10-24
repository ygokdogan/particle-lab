#include <algorithm>
#include <stdexcept>

#include "sandbox_lab/Grid.hpp"
#include "sandbox_lab/MaterialProperties.hpp"

namespace sandbox {
    Grid::Grid(int width, int height) : width(width), height(height), cells(width * height) {}

    Cell &Grid::getCell(int x, int y) {
        if (!isValidCell(x, y)) {
            throw std::out_of_range("Cell coordinates are out of bounds");
        }

        int index = y * width + x;
        return cells[index];
    }

    const Cell &Grid::getCell(int x, int y) const {
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

    bool Grid::eraseCell(int x, int y) {
        if (!isValidCell(x, y)) {
            return false;
        }

        getCell(x, y) = Cell{};
        return true;
    }

    bool Grid::isValidCell(int x, int y) const {
        return (x >= 0 && x < width && y >= 0 && y < height);
    }

    bool Grid::isEmptyCell(int x, int y) const {
        if (!isValidCell(x, y)) {
            return false;
        }

        const Cell &cell = getCell(x, y);
        return cell.solid == Material::Empty && cell.liquids.empty();
    }

    bool Grid::placeSolid(int x, int y, Material material) {
        if (!isValidCell(x, y) || !isEmptyCell(x, y)) {
            return false;
        }

        if (material == Material::Empty) {
            return false;
        }

        Behaviour behaviour = getMaterialProperties(material).behaviour;
        if (behaviour != Behaviour::Powder && behaviour != Behaviour::Static) {
            return false;
        }

        Cell& cell = getCell(x, y);
        cell.solid = material;
        cell.velocityY = 0.0f;
        cell.movementRemainder = 0.0f;
        return true;
    }

    LiquidLayer *Grid::getLiquidLayer(int x, int y, Material material) {
        for (LiquidLayer &layer : getCell(x, y).liquids) {
            if (layer.material == material) {
                return &layer;
            }
        }
        return nullptr;
    }

    float Grid::getTotalLiquidAmount(int x, int y) const {
        if (!isValidCell(x, y)) {
            return 0.0f;
        }

        float amount = 0.0f;
        for (const LiquidLayer &layer : getCell(x, y).liquids) {
            amount += layer.amount;
        }

        return amount;
    }

    float Grid::getLiquidAmount(int x, int y, Material material) const {
        if (!isValidCell(x, y)) {
            return 0.0f;
        }

        for (const LiquidLayer &layer : getCell(x, y).liquids) {
            if (layer.material == material) {
                return layer.amount;
            }
        }

        return 0.0f;
    }

    float Grid::addLiquid(int x, int y, Material material, float amount, float velocityY) {
        if (!isValidCell(x, y)) 
            return 0.0f;
        if (getCell(x, y).solid != Material::Empty || getMaterialProperties(material).behaviour != Behaviour::Liquid) 
            return 0.0f;
        if (amount <= 0.0f) 
            return 0.0f;

        LiquidLayer* layer = getLiquidLayer(x, y, material);
        float availableCapacity = 1.0f - getTotalLiquidAmount(x, y);
        float addedAmount = std::min(amount, availableCapacity);

        if (addedAmount <= 0.0f) 
            return 0.0f;

        if (layer != nullptr) {
            float newAmount = layer->amount + addedAmount;
            layer->velocityY = (layer->velocityY * layer->amount + velocityY * addedAmount) / newAmount;
            layer->amount = newAmount;
            return addedAmount;
        }

        getCell(x, y).liquids.push_back({material, addedAmount, velocityY});
        return addedAmount;
    }

    float Grid::removeLiquid(int x, int y, Material material, float amount) {
        if (!isValidCell(x, y)) {
            return 0.0f;
        }

        if (getCell(x, y).solid != Material::Empty || getMaterialProperties(material).behaviour != Behaviour::Liquid) {
            return 0.0f;
        }

        if (amount <= 0) {
            return 0.0f;
        }

        auto &liquids = getCell(x, y).liquids;
        for (auto it = liquids.begin(); it != liquids.end(); ++it) {
            if (it->material == material) {
                float availableLiquid = it->amount;
                float removedAmount = std::min(availableLiquid, amount);

                it->amount -= removedAmount;
                if (it->amount <= 0) {
                    liquids.erase(it);
                }
                return removedAmount;
            }
        }
        return 0.0f;
    }
} // namespace sandbox