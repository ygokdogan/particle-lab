#include <algorithm>

#include "sandbox_lab/LiquidProperties.hpp"
#include "sandbox_lab/LiquidPhysics.hpp"
#include "sandbox_lab/MaterialProperties.hpp"

namespace sandbox {
    LiquidPhysics::LiquidPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection) : rng(rng), randomDirection(randomDirection) {}

    void LiquidPhysics::updateLiquid(Grid& grid, int x, int y, Material material, uint16_t frame) {
        LiquidLayer* layer = grid.getLiquidLayer(x, y, material);
        if (!layer || layer->lastUpdatedFrame == frame) return;
        if (fallLiquid(grid, x, y, material, frame)) return;

        const LiquidProperties& properties = getLiquidProperties(material);
        
        float amount = grid.getLiquidAmount(x, y, material);
        if (amount <= 0.0f) {
            return;
        }

        transferLiquid(grid, x, y, MoveDirection::Down,material, amount * properties.verticalFlow);
        amount = grid.getLiquidAmount(x, y, material);

        if (amount > 0.0f) {
            displaceLiquid(grid, x, y, MoveDirection::Down,material, amount * properties.verticalFlow);
        }

        amount = grid.getLiquidAmount(x, y, material);
        if (amount <= 0.001f) {
            return;
        }

        bool canFlowLeft = grid.isValidCell(x - 1, y) && grid.getCell(x - 1, y).solid == Material::Empty;
        bool canFlowRight = grid.isValidCell(x + 1, y) && grid.getCell(x + 1, y).solid == Material::Empty;

        bool leftFirst = randomDirection(rng) == 0;
        MoveDirection firstDirection = leftFirst ? MoveDirection::DownLeft : MoveDirection::DownRight;
        MoveDirection secondDirection = leftFirst ? MoveDirection::DownRight : MoveDirection::DownLeft;

        bool canFlowFirst = leftFirst ? canFlowLeft : canFlowRight;
        bool canFlowSecond = leftFirst ? canFlowRight : canFlowLeft;

        if (canFlowFirst) {
            amount = grid.getLiquidAmount(x, y, material);
            transferLiquid(grid, x, y, firstDirection,material, amount * properties.diagonalFlow);

            amount = grid.getLiquidAmount(x, y, material);
            displaceLiquid(grid, x, y, firstDirection, material, amount * properties.diagonalFlow);
        }

        if (canFlowSecond) {
            amount = grid.getLiquidAmount(x, y, material);
            transferLiquid(grid, x, y, secondDirection,material, amount * properties.diagonalFlow);

            amount = grid.getLiquidAmount(x, y, material);
            displaceLiquid(grid, x, y, secondDirection,material, amount * properties.diagonalFlow);
        }

        bool balanceLeftFirst = randomDirection(rng) == 0;
        MoveDirection firstHorizontal = balanceLeftFirst ? MoveDirection::Left : MoveDirection::Right;
        MoveDirection secondHorizontal = balanceLeftFirst ? MoveDirection::Right : MoveDirection::Left;

        spreadLiquid(grid, x, y, firstHorizontal, material);
        spreadLiquid(grid, x, y, secondHorizontal, material);
    }

    float LiquidPhysics::transferLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material, float amount) {
        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) || !grid.isValidCell(targetX, targetY)) {
            return 0.0f;
        }

        if (grid.getCell(x, y).solid != Material::Empty) {
            return 0.0f;
        }

        if (amount <= 0.0f) {
            return 0.0f;
        }

        bool reactivePair = (material == Material::Water && grid.getLiquidAmount(targetX, targetY, Material::Lava) > 0.0f) ||
                            (material == Material::Lava && grid.getLiquidAmount(targetX, targetY, Material::Water) > 0.0f);

        if (reactivePair) {
            return 0.0f;
        }

        float availableSource = grid.getLiquidAmount(x, y, material);
        float availableCapacity = 1.0f - grid.getTotalLiquidAmount(targetX, targetY);
        float transferableAmount = std::min({amount, availableCapacity, availableSource});

        float sourceVelocityY = 0.0f;
        for (const LiquidLayer& layer : grid.getCell(x, y).liquids) {
            if (layer.material == material) {
                sourceVelocityY = layer.velocityY;
                break;
            }
        }

        float addedAmount = grid.addLiquid(targetX, targetY, material, transferableAmount, sourceVelocityY);
        grid.removeLiquid(x, y, material, addedAmount);

        return addedAmount;
    }

    float LiquidPhysics::balanceLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material) {
        if (direction != MoveDirection::Left && direction != MoveDirection::Right) {
            return 0.0f;
        }

        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) || !grid.isValidCell(targetX, targetY)) {
            return 0.0f;
        }

        if (grid.getCell(targetX, targetY).solid != Material::Empty) {
            return 0.0f;
        }

        float sourceAmount = grid.getLiquidAmount(x, y, material);
        float targetAmount = grid.getLiquidAmount(targetX, targetY, material);
        float diff = sourceAmount - targetAmount;

        if (diff <= 0.001f) {
            return 0.0f;
        }

        float flowAmount = (diff / 2.0f) * getLiquidProperties(material).horizontalFlow;
        float transferred = transferLiquid(grid, x, y, direction, material, flowAmount);
        float remaining = flowAmount - transferred;

        float displaced = 0.0f;

        if (remaining > 0.001f) {
            displaced = displaceLiquid(grid, x, y, direction, material, remaining);
        }

        return transferred + displaced;
    }

    void LiquidPhysics::spreadLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material) {
        if (direction != MoveDirection::Left && direction != MoveDirection::Right) {
            return;
        }

        int step = direction == MoveDirection::Left ? -1 : 1;
        for (int i = 0; i < horizontalFlowDistance; ++i) {
            int currentX = x + i * step;

            if (!grid.isValidCell(currentX, y)) {
                break;
            }

            if (grid.getCell(currentX, y).solid != Material::Empty) {
                break;
            }

            float transferred = balanceLiquid(grid, currentX, y, direction, material);

            if (transferred <= 0.0f) {
                break;
            }
        }
    }

    float LiquidPhysics::displaceLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material, float amount) {
        if (amount <= 0.0f) {
            return 0.0f;
        }

        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) || !grid.isValidCell(targetX, targetY)) {
            return 0.0f;
        }

        Cell &source = grid.getCell(x, y);
        Cell &target = grid.getCell(targetX, targetY);

        if (source.solid != Material::Empty || target.solid != Material::Empty) {
            return 0.0f;
        }

        float sourceDensity = getMaterialProperties(material).density;
        Material displacedMaterial = Material::Empty;
        float lowestDensity = sourceDensity;

        for (const LiquidLayer &layer : target.liquids) {
            float density = getMaterialProperties(layer.material).density;

            if (density < lowestDensity) {
                lowestDensity = density;
                displacedMaterial = layer.material;
            }
        }

        if (displacedMaterial == Material::Empty) {
            return 0.0f;
        }

        if (material == Material::Water && grid.getLiquidAmount(targetX, targetY, Material::Lava) > 0.0f) {
            return 0.0f;
        }

        if (material == Material::Lava && grid.getLiquidAmount(targetX, targetY, Material::Water) > 0.0f) {
            bool downward = direction == MoveDirection::Down || direction == MoveDirection::DownLeft || direction == MoveDirection::DownRight;

            if (downward) {
                bool leftFirst = randomDirection(rng) == 0;

                MoveDirection first = leftFirst ? MoveDirection::Left : MoveDirection::Right;

                MoveDirection second = leftFirst ? MoveDirection::Right : MoveDirection::Left;

                float waterAmount = grid.getLiquidAmount(targetX, targetY, Material::Water);

                float moved = transferLiquid(grid, targetX, targetY, first, Material::Water, waterAmount);

                moved += transferLiquid(grid, targetX, targetY, second, Material::Water, waterAmount - moved);

                if (grid.getLiquidAmount(targetX, targetY, Material::Water) <= 0.0001f) {
                    return transferLiquid(grid, x, y, direction, Material::Lava, amount);
                }
            }

            return 0.0f;
        }

        float sourceAmount = grid.getLiquidAmount(x, y, material);
        float targetAmount = grid.getLiquidAmount(targetX, targetY, displacedMaterial);

        float totalMoved = 0.0f;
        float remainingAmount = std::min(amount, sourceAmount);

        if (direction == MoveDirection::Down || direction == MoveDirection::DownLeft || direction == MoveDirection::DownRight) {
            bool leftFirst = randomDirection(rng) == 0;
            MoveDirection firstDirection = leftFirst ? MoveDirection::Left : MoveDirection::Right;
            MoveDirection secondDirection = leftFirst ? MoveDirection::Right : MoveDirection::Left;
            float requested = std::min(remainingAmount, targetAmount);
            float movedAside = transferLiquid(grid, targetX, targetY, firstDirection, displacedMaterial, requested / 2.0f);
            movedAside += transferLiquid(grid, targetX, targetY, secondDirection, displacedMaterial, requested - movedAside);
            movedAside += transferLiquid(grid, targetX, targetY, firstDirection, displacedMaterial, requested - movedAside);
            float movedDown = transferLiquid(grid, x, y, direction, material, movedAside);
            remainingAmount -= movedDown;
            totalMoved += movedDown;
        }

        sourceAmount = grid.getLiquidAmount(x, y, material);
        targetAmount = grid.getLiquidAmount(targetX, targetY, displacedMaterial);

        float exchangeAmount = std::min({amount, sourceAmount, targetAmount});

        if (exchangeAmount <= 0.0f) {
            return totalMoved;
        }

        grid.removeLiquid(x, y, material, exchangeAmount);
        grid.removeLiquid(targetX, targetY, displacedMaterial, exchangeAmount);

        grid.addLiquid(x, y, displacedMaterial, exchangeAmount);
        grid.addLiquid(targetX, targetY, material, exchangeAmount);

        return totalMoved + exchangeAmount;
    }

    void LiquidPhysics::displaceLiquidSideways(Grid &grid, int x, int y) {
        if (!grid.isValidCell(x, y)) {
            return;
        }

        std::vector<Material> materials;

        for (const LiquidLayer &layer : grid.getCell(x, y).liquids) {
            materials.push_back(layer.material);
        }

        bool leftFirst = randomDirection(rng) == 0;
        MoveDirection firstDirection = leftFirst ? MoveDirection::Left : MoveDirection::Right;
        MoveDirection secondDirection = leftFirst ? MoveDirection::Right : MoveDirection::Left;

        for (Material material : materials) {
            float amount = grid.getLiquidAmount(x, y, material);

            if (amount <= 0.0f) {
                continue;
            }

            transferLiquid(grid, x, y, firstDirection, material, amount / 2.0f);
            amount = grid.getLiquidAmount(x, y, material);

            transferLiquid(grid, x, y, secondDirection, material, amount);

            amount = grid.getLiquidAmount(x, y, material);

            if (amount > 0.0f) {
                transferLiquid(grid, x, y, firstDirection, material, amount);
            }
        }
    }

    bool LiquidPhysics::fallLiquid(Grid &grid, int x, int y, Material material, std::uint16_t frame) {
        LiquidLayer* bottom = grid.getLiquidLayer(x, y, material);

        if (!bottom || bottom->lastUpdatedFrame == frame) return false;
        if (grid.isValidCell(x, y+1) && grid.getLiquidAmount(x, y+1, material) > 0.0f) return false;

        int topY = y;
        while (topY > 0) {
            const Cell& cell = grid.getCell(x, topY - 1);
            if (cell.solid != Material::Empty || cell.liquids.size() != 1) break;
            if (cell.liquids.front().material != material) break;
            if (cell.liquids.front().lastUpdatedFrame == frame) break;
            --topY;
        }

        const LiquidProperties props = getLiquidProperties(material);
        float velocityY = 0.0f;

        for (int row = topY; row <= y; row++) {
            LiquidLayer* layer = grid.getLiquidLayer(x, row, material);
            velocityY = std::max(velocityY, layer->velocityY);
        }

        if (!grid.isEmptyCell(x, y+1)) {
            for (int row = topY; row <= y; row++) {
                grid.getLiquidLayer(x, row, material)->velocityY = 0.0f;
            }

            applyImpactSpread(grid, x, y, material, velocityY);
            return false;
        }

        velocityY = std::min(velocityY + props.gravity, props.maxVelocity);
        int requestedSteps = std::max(1, static_cast<int>(velocityY));
        int steps = 0;

        for (int i = 1; i <= requestedSteps; i++) {
            if (!grid.isEmptyCell(x, y + i)) break;
            ++steps;
        }

        if (steps == 0) return false;

        for (int row = y; row >= topY; row--) {
            std::swap(grid.getCell(x, row).liquids, grid.getCell(x, row + steps).liquids);
            LiquidLayer* moved = grid.getLiquidLayer(x, row + steps, material);
            moved->velocityY = velocityY;
            moved->lastUpdatedFrame = frame;
        }

        if (!grid.isEmptyCell(x, y + steps + 1)) {
            for (int row = topY + steps; row <= y + steps; ++row) {
                grid.getLiquidLayer(x, row, material)->velocityY = 0.0f;
            }
            applyImpactSpread(grid, x, y + steps, material, velocityY);
        }

        return true;
    }

    void LiquidPhysics::applyImpactSpread(Grid& grid, int x, int y, Material material, float impactSpeed) {
        const LiquidProperties& properties = getLiquidProperties(material);
        if (impactSpeed < properties.minImpactSpeed) return;

        int passes = std::clamp(static_cast<int>(impactSpeed / properties.impactSpeedPerPass), 1, properties.maxImpactPasses);

        for (int i = 0; i < passes; ++i) {
            bool leftFirst = randomDirection(rng) == 0;

            for (int side = 0; side < 2; side++) {
                bool left = side == 0 ? leftFirst : !leftFirst;
                int dx = left ? -1 : 1;
                MoveDirection diagonal = left ? MoveDirection::DownLeft : MoveDirection::DownRight;
                MoveDirection horizontal = left ? MoveDirection::Left : MoveDirection::Right;

                for (int distance = horizontalFlowDistance; distance >= 1; --distance) {
                    int sourceX = x + (distance - 1) * dx;
                    int targetX = sourceX + dx;

                    if (!grid.isValidCell(sourceX, y) || !grid.isValidCell(targetX, y)) continue;
                    if (grid.getCell(targetX, y).solid != Material::Empty) continue;

                    float amount = grid.getLiquidAmount(sourceX, y, material);
                    if (amount <= 0.0f) continue;

                    float moved = 0.0f;
                    if (grid.isValidCell(targetX, y+1)) {
                        moved = transferLiquid(grid, sourceX, y, diagonal, material, amount);
                    }

                    if (moved <= 0.0f) {
                        transferLiquid(grid, sourceX, y, horizontal, material, amount);
                    }
                }
            }
        }
    }
}
