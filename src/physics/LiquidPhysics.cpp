#include "sandbox_lab/LiquidPhysics.hpp"
#include "sandbox_lab/MaterialProperties.hpp"
#include <algorithm>

namespace sandbox {
    struct LiquidFlow {
        float vertical;
        float diagonal;
        float horizontal;
    };

    LiquidFlow getLiquidFlow(Material material) {
        switch (material) {
            case Material::Water:
                return {1.0f, 1.0f, 1.0f};

            case Material::Oil:
                return {1.0f, 0.6f, 0.5f};

            case Material::Lava:
                return {1.0f, 0.35f, 0.25f};

            default:
                return {1.0f, 1.0f, 1.0f};
        }
    }

    LiquidPhysics::LiquidPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection) : rng(rng), randomDirection(randomDirection) {}

    void LiquidPhysics::updateLiquid(Grid& grid, int x, int y, Material material) {
        if (fallLiquid(grid, x, y, material)) {
            return;
        }

        LiquidFlow flow = getLiquidFlow(material);

        float amount = grid.getLiquidAmount(x, y, material);

        if (amount <= 0.0f) {
            return;
        }

        transferLiquid(grid, x, y, MoveDirection::Down,material, amount * flow.vertical);
        amount = grid.getLiquidAmount(x, y, material);

        if (amount > 0.0f) {
            displaceLiquid(grid, x, y, MoveDirection::Down,material, amount * flow.vertical);
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
            transferLiquid(grid, x, y, firstDirection,material, amount * flow.diagonal);

            amount = grid.getLiquidAmount(x, y, material);
            displaceLiquid(grid, x, y, firstDirection, material, amount * flow.diagonal);
        }

        if (canFlowSecond) {
            amount = grid.getLiquidAmount(x, y, material);
            transferLiquid(grid, x, y, secondDirection,material, amount * flow.diagonal);

            amount = grid.getLiquidAmount(x, y, material);
            displaceLiquid(grid, x, y, secondDirection,material, amount * flow.diagonal);
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

        float flowAmount = (diff / 2.0f) * getLiquidFlow(material).horizontal;
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

    bool LiquidPhysics::fallLiquid(Grid &grid, int x, int y, Material material) {
        if (!grid.isValidCell(x, y+1)) {
            return false;
        }

        Cell &below = grid.getCell(x, y+1);
        if (below.solid != Material::Empty || !below.liquids.empty()) {
            for (LiquidLayer& layer : grid.getCell(x, y).liquids) {
                if (layer.material == material) {
                    layer.velocityY = 0.0f;
                    break;
                }
            }

            return false;
        }
        float gravity = 0.4f;
        float maxVelocity = 5.0f;
        float velocityY = 0.0f;

        switch (material) {
            case Material::Water:
                gravity = 0.4f;
                maxVelocity = 3.5f;
                break;
            case Material::Oil:
                gravity = 0.35f;
                maxVelocity = 3.0f;
                break;
            case Material::Lava:
                gravity = 0.2f;
                maxVelocity = 2.0f;
                break;
            default:
                break;
        }
        
        for (LiquidLayer& layer : grid.getCell(x, y).liquids) {
            if (layer.material == material) {
                layer.velocityY = std::min(layer.velocityY + gravity, maxVelocity);
                velocityY = layer.velocityY;
                break;
            }
        }

        int steps = std::max(1, static_cast<int>(velocityY));
        int currentY = y;

        for (int i = 0; i < steps; ++i) {
            int nextY = currentY + 1;

            if (!grid.isValidCell(x, nextY)) {
                break;
            }

            const Cell& target = grid.getCell(x, nextY);
            if (target.solid != Material::Empty || !target.liquids.empty()) {
                break;
            }

            float amount = grid.getLiquidAmount(x, currentY, material);
            if (amount <= 0.0f) {
                break;
            }

            float moved = transferLiquid(grid,x,currentY,MoveDirection::Down,material,amount);
            if (moved <= 0.0f) {
                break;
            }

            ++currentY;
            if (moved < amount - 0.0001f) {
                break;
            }
        }

        if (currentY != y) {
            bool blocked = !grid.isValidCell(x, currentY + 1);

            if (!blocked) {
                const Cell& next = grid.getCell(x, currentY + 1);
                blocked = next.solid != Material::Empty || !next.liquids.empty();
            }

            if (blocked) {
                for (LiquidLayer& layer :
                    grid.getCell(x, currentY).liquids) {
                    if (layer.material == material) {
                        layer.velocityY = 0.0f;
                        break;
                    }
                }
            }
        }

        return currentY != y;
    }
}
