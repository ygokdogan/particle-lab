#include <algorithm>
#include <array>
#include <utility>
#include <random>

#include "sandbox_lab/Simulation.hpp"
#include "sandbox_lab/MaterialProperties.hpp"

namespace sandbox
{
    void Simulation::update(Grid &grid) {
        currentFrame++;
        int startX = scanLeftToRight ? 0 : grid.getWidth()-1;
        int endX = scanLeftToRight ? grid.getWidth() : -1;
        int stepX = scanLeftToRight ? 1 : -1;

        for (int y = grid.getHeight() - 1; y >= 0; --y) {
            for (int x = startX; x != endX; x += stepX) {
                Cell& cell = grid.getCell(x, y);

                if (cell.gas != Material::Empty && cell.lastGasUpdatedFrame != currentFrame) {
                    updateGas(grid, x, y);
                }

                if (grid.getLiquidAmount(x, y, Material::Lava) > 0.0f) {
                    updateLava(grid, x, y);
                }

                std::vector<Material> materials;    

                for (const LiquidLayer& layer : cell.liquids) {
                    materials.push_back(layer.material);
                }
                for (Material material : materials) {
                    if (grid.getLiquidAmount(x, y, material) > 0.0f) {
                        updateLiquid(grid, x, y, material);
                    }
                }

                if (cell.lastUpdatedFrame == currentFrame){
                    continue;
                }

                const MaterialProperties& properties = getMaterialProperties(cell.solid);

                switch (properties.behaviour) {
                    case Behaviour::Powder:
                        if (cell.solid == Material::CoolingRock) {
                            bool canFall = canMoveSolid(grid, x, y, MoveDirection::Down);

                            bool canFallLeft = canMoveSolid(grid, x, y, MoveDirection::DownLeft) && canMoveSolid(grid, x, y, MoveDirection::Left);

                            bool canFallRight = canMoveSolid(grid, x, y, MoveDirection::DownRight) && canMoveSolid(grid, x, y, MoveDirection::Right);

                            if (!canFall && !canFallLeft && !canFallRight) {
                                cell.solid = Material::Stone;
                                break;
                            }
                        }
                        updatePowder(grid, x, y);
                        break;
                    case Behaviour::Static:
                        if (cell.solid == Material::Wood) {
                            updateWood(grid, x ,y);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        coolLava(grid);
        scanLeftToRight = !scanLeftToRight;
    }

    void Simulation::updatePowder(Grid &grid, int x, int y)
    {
        if (moveSolid(grid, x, y, MoveDirection::Down)) {
            return;
        }

        bool canMoveLeft = canMoveSolid(grid, x, y, MoveDirection::DownLeft) && canMoveSolid(grid, x, y, MoveDirection::Left);
        bool canMoveRight = canMoveSolid(grid, x, y, MoveDirection::DownRight) && canMoveSolid(grid, x, y, MoveDirection::Right);

        if (canMoveLeft && canMoveRight) {
            bool moveLeft = randomDirection(rng) == 0;

            if (moveLeft) {
                moveSolid(grid, x, y, MoveDirection::DownLeft);
            }
            else {
                moveSolid(grid, x, y, MoveDirection::DownRight);
            }
            return;
        }

        if (canMoveRight) {
            moveSolid(grid, x, y, MoveDirection::DownRight);
            return;
        }

        if (canMoveLeft) {
            moveSolid(grid, x, y, MoveDirection::DownLeft);
            return;
        }
    }

    void Simulation::updateGas(Grid& grid, int x, int y) {
        Cell& cell = grid.getCell(x, y);

        if (cell.gas == Material::Empty ||
            cell.lastGasUpdatedFrame == currentFrame) {
            return;
        }

        Material gas = cell.gas;

        cell.lastGasUpdatedFrame = currentFrame;

        if (cell.gasLifetime <= 1) {
            cell.gas = Material::Empty;
            cell.gasLifetime = 0;
            return;
        }

        --cell.gasLifetime;

        int steps = gas == Material::Steam ? 2 : 1;

        for (int step = 0; step < steps; ++step) {
            bool leftFirst = randomDirection(rng) == 0;

            std::array<MoveDirection, 5> directions{
                MoveDirection::Up,
                leftFirst ? MoveDirection::UpLeft : MoveDirection::UpRight,
                leftFirst ? MoveDirection::UpRight : MoveDirection::UpLeft,
                leftFirst ? MoveDirection::Left : MoveDirection::Right,
                leftFirst ? MoveDirection::Right : MoveDirection::Left
            };

            bool moved = false;

            for (MoveDirection direction : directions) {
                if (moveGas(grid, x, y, direction)) {
                    auto [nx, ny] = getTargetPosition(x, y, direction);
                    x = nx;
                    y = ny;
                    moved = true;
                    break;
                }
            }

            if (!moved) {
                break;
            }
        }
    }

    void Simulation::updateLiquid(Grid& grid, int x, int y, Material material) {
        float amount = grid.getLiquidAmount(x, y, material);

        transferLiquid(grid, x, y, MoveDirection::Down, material, amount);

        amount = grid.getLiquidAmount(x, y, material);
        if (amount <= 0.0f) return;

        displaceLiquid(grid, x, y, MoveDirection::Down, material, amount);

        amount = grid.getLiquidAmount(x, y, material);
        if (amount <= 0.0f) return;

        bool canFlowLeft = grid.isValidCell(x - 1, y) && grid.getCell(x - 1, y).solid == Material::Empty;
        bool canFlowRight = grid.isValidCell(x + 1, y) && grid.getCell(x + 1, y).solid == Material::Empty;
        bool leftFirst = randomDirection(rng) == 0;

        MoveDirection firstDirection = leftFirst ? MoveDirection::DownLeft : MoveDirection::DownRight;

        MoveDirection secondDirection = leftFirst ? MoveDirection::DownRight : MoveDirection::DownLeft;

        bool canFlowFirst = leftFirst ? canFlowLeft : canFlowRight;
        bool canFlowSecond = leftFirst ? canFlowRight : canFlowLeft;

        if (canFlowFirst) {
            transferLiquid(grid, x, y, firstDirection, material, amount / 2.0f);

            amount = grid.getLiquidAmount(x, y, material);
            displaceLiquid(grid, x, y, firstDirection, material, amount);
        }

        if (canFlowSecond) {
            amount = grid.getLiquidAmount(x, y, material);
            transferLiquid(grid, x, y, secondDirection, material, amount);

            amount = grid.getLiquidAmount(x, y, material);
            displaceLiquid(grid, x, y, secondDirection, material, amount);
        }

        if (canFlowFirst) {
            amount = grid.getLiquidAmount(x, y, material);
            transferLiquid(grid, x, y, firstDirection, material, amount);
        }

        bool balanceLeftFirst = randomDirection(rng) == 0;

        MoveDirection firstHorizontal = balanceLeftFirst ? MoveDirection::Left : MoveDirection::Right;

        MoveDirection secondHorizontal = balanceLeftFirst ? MoveDirection::Right : MoveDirection::Left;

        spreadLiquid(grid, x, y, firstHorizontal, material);
        spreadLiquid(grid, x, y, secondHorizontal, material);
    }

    bool Simulation::moveSolid(Grid &grid, int x, int y, MoveDirection direction)
    {
        if (!canMoveSolid(grid, x, y, direction)) {
            return false;
        }

        auto [targetX, targetY] = getTargetPosition(x, y, direction);
        
        Cell& source = grid.getCell(x, y);
        Cell& target = grid.getCell(targetX, targetY);

        if (!target.liquids.empty()) {
            displaceLiquidSideways(grid, targetX, targetY);
        }

        source.lastUpdatedFrame = currentFrame;
        std::swap(source, target);
        return true;
    }

    float Simulation::transferLiquid(Grid & grid, int x, int y, MoveDirection direction, Material material, float amount) {
        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) || !grid.isValidCell(targetX, targetY)) {
            return 0.0f;
        }

        if(grid.getCell(x, y).solid != Material::Empty) {
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

        float addedAmount = grid.addLiquid(targetX, targetY, material, transferableAmount);
        grid.removeLiquid(x, y, material, addedAmount);

        return addedAmount;
    }

    float Simulation::balanceLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material) {
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

        float flowAmount = diff / 2.0f;
        float transferred = transferLiquid(grid, x, y, direction, material, flowAmount);
        float remaining = flowAmount - transferred;
        
        float displaced = 0.0f;

        if (remaining > 0.001f) {
            displaced = displaceLiquid(grid, x, y, direction, material, remaining);
        }

        return transferred + displaced;
    }

    void Simulation::spreadLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material)
    {
        if (direction != MoveDirection::Left &&
            direction != MoveDirection::Right) {
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

    float Simulation::displaceLiquid(Grid &grid, int x, int y, MoveDirection direction, Material material, float amount) {
        if (amount <= 0.0f) {
            return 0.0f;
        }

        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) ||
            !grid.isValidCell(targetX, targetY)) {
            return 0.0f;
        }

        Cell& source = grid.getCell(x, y);
        Cell& target = grid.getCell(targetX, targetY);

        if (source.solid != Material::Empty || target.solid != Material::Empty) {
            return 0.0f;
        }

        float sourceDensity = getMaterialProperties(material).density;
        Material displacedMaterial = Material::Empty;
        float lowestDensity = sourceDensity;

        for (const LiquidLayer& layer : target.liquids) {
            float density = getMaterialProperties(layer.material).density;

            if (density < lowestDensity) {
                lowestDensity = density;
                displacedMaterial = layer.material;
            }
        }

        if (displacedMaterial == Material::Empty) {
            return 0.0f;
        }

        if (material == Material::Water &&
            grid.getLiquidAmount(targetX, targetY, Material::Lava) > 0.0f) {
            return 0.0f;
        }

        if (material == Material::Lava &&
            grid.getLiquidAmount(targetX, targetY, Material::Water) > 0.0f) {
            bool downward = direction == MoveDirection::Down || direction == MoveDirection::DownLeft || direction == MoveDirection::DownRight;

            if (downward) {
                bool leftFirst = randomDirection(rng) == 0;

                MoveDirection first = leftFirst ? MoveDirection::Left : MoveDirection::Right;

                MoveDirection second = leftFirst ? MoveDirection::Right : MoveDirection::Left;

                float waterAmount = grid.getLiquidAmount(targetX, targetY, Material::Water);

                float moved = transferLiquid(
                    grid, targetX, targetY, first, Material::Water, waterAmount
                );

                moved += transferLiquid(
                    grid, targetX, targetY, second, Material::Water,
                    waterAmount - moved
                );

                if (grid.getLiquidAmount(targetX, targetY, Material::Water) <= 0.0001f) {
                    return transferLiquid(
                        grid, x, y, direction, Material::Lava, amount
                    );
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

    void Simulation::displaceLiquidSideways(Grid& grid, int x, int y) {
        if (!grid.isValidCell(x, y)) {
            return;
        }

        std::vector<Material> materials;

        for (const LiquidLayer& layer : grid.getCell(x, y).liquids) {
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

            transferLiquid(grid, x, y, secondDirection, material,amount);

            amount = grid.getLiquidAmount(x, y, material);

            if (amount > 0.0f) {
                transferLiquid(grid, x, y, firstDirection, material, amount);
            }
        }
    }

    void Simulation::updateWood(Grid& grid, int x, int y) {
        constexpr std::array<std::pair<int, int>, 8> neighbors{{
            {-1, -1}, {0, -1}, {1, -1},
            {-1,  0},          {1,  0},
            {-1,  1}, {0,  1}, {1,  1}
        }};

        Cell& cell = grid.getCell(x, y);

        if (cell.burnTicks == 0) {
            std::bernoulli_distribution ignitionChance(0.02);

            for (auto [dx, dy] : neighbors) {
                int nx = x + dx;
                int ny = y + dy;

                if (!grid.isValidCell(nx, ny)) {
                    continue;
                }

                if (grid.getLiquidAmount(nx, ny, Material::Lava) >= 0.25f &&
                    ignitionChance(rng)) {
                    igniteWood(grid, x, y);
                    return;
                }
            }
            return;
        }

        if (std::bernoulli_distribution(0.08)(rng)) {
            spawnSmoke(grid, x, y);
        }

        std::bernoulli_distribution spreadChance(0.012);

        for (auto [dx, dy] : neighbors) {
            int nx = x + dx;
            int ny = y + dy;

            if (!grid.isValidCell(nx, ny)) {
                continue;
            }

            const Cell& neighbor = grid.getCell(nx, ny);

            if (neighbor.solid == Material::Wood &&
                neighbor.burnTicks == 0 &&
                spreadChance(rng)) {
                igniteWood(grid, nx, ny);
            }
        }

        ++cell.burnTicks;

        if (cell.burnTicks >= cell.burnDuration) {
            grid.eraseCell(x, y);
        }
    }

    void Simulation::updateLava(Grid &grid, int x, int y) {
        float lavaAmount = grid.getLiquidAmount(x, y, Material::Lava);

        if (lavaAmount <= 0.25f) {
            return;
        }

        std::array<std::pair<int, int>, 4> offsets{{
            {0, -1},
            {0, 1},
            {-1, 0},
            {1, 0}
        }};

        std::shuffle(offsets.begin(), offsets.end(), rng);

        std::bernoulli_distribution sandMeltChance(0.75);
        std::bernoulli_distribution oilBurnChance(0.95);

        float oilHere = grid.getLiquidAmount(x, y, Material::Oil);

        if (oilHere > 0.0f && oilBurnChance(rng)) {
            grid.removeLiquid(x, y, Material::Oil, oilHere);
        }

        for (auto [dx, dy] : offsets) {
            int nx = x + dx;
            int ny = y + dy;

            if (!grid.isValidCell(nx, ny)) {
                continue;
            }

            Cell& neighbor = grid.getCell(nx, ny);

            if (neighbor.solid == Material::Sand) {
                if (sandMeltChance(rng)) {
                    grid.eraseCell(nx, ny);
                }
                continue;
            }

            float oilAmount = grid.getLiquidAmount(nx, ny, Material::Oil);

            if (oilAmount > 0.0f && oilBurnChance(rng)) {
                grid.removeLiquid(nx, ny, Material::Oil, oilAmount);
            }
        }
    }

    void Simulation::igniteWood(Grid& grid, int x, int y) {
        if (!grid.isValidCell(x, y)) {
            return;
        }

        Cell& cell = grid.getCell(x, y);

        if (cell.solid != Material::Wood || cell.burnTicks != 0) {
            return;
        }

        cell.burnTicks = 1;
        cell.burnDuration = std::uniform_int_distribution<int>(100, 240)(rng);
    }

    void Simulation::spawnSmoke(Grid &grid, int x, int y) {
        std::array<std::pair<int, int>, 5> offsets{{
            {0, -1},
            {-1, -1},
            {1, -1},
            {-1, 0},
            {1, 0}
        }};

        std::shuffle(offsets.begin(), offsets.end(), rng);

        for (auto [dx, dy] : offsets) {
            int targetX = x + dx;
            int targetY = y + dy;

            if (!grid.isValidCell(targetX, targetY)) {
                continue;
            }

            Cell& target = grid.getCell(targetX, targetY);

            if (target.solid != Material::Empty || !target.liquids.empty() || target.gas != Material::Empty) {
                continue;
            }

            target.gas = Material::Smoke;
            target.gasLifetime = std::uniform_int_distribution<int>(240, 350)(rng);
            target.lastGasUpdatedFrame = currentFrame;

            return;
        }
    }

    bool Simulation::moveGas(Grid& grid, int x, int y, MoveDirection direction) {
        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) ||
            !grid.isValidCell(targetX, targetY)) {
            return false;
        }

        Cell& source = grid.getCell(x, y);
        Cell& target = grid.getCell(targetX, targetY);

        if (source.gas == Material::Empty || target.gas != Material::Empty || target.solid != Material::Empty) {
            return false;
        }

        if (source.gas == Material::Smoke && !target.liquids.empty()) {
            return false;
        }

        if (source.gas == Material::Steam) {
            float totalLiquid = grid.getTotalLiquidAmount(targetX, targetY);
            float waterAmount = grid.getLiquidAmount(targetX, targetY, Material::Water);

            if (totalLiquid - waterAmount > 0.001f) {
                return false;
            }
        }

        target.gas = source.gas;
        target.gasLifetime = source.gasLifetime;
        target.lastGasUpdatedFrame = currentFrame;

        source.gas = Material::Empty;
        source.gasLifetime = 0;

        return true;
    }

    void Simulation::coolLava(Grid& grid) {
        constexpr std::array<std::pair<int, int>, 4> offsets{{
            {0, -1}, {0, 1}, {-1, 0}, {1, 0}
        }};

        for (int y = 0; y < grid.getHeight(); ++y) {
            for (int x = 0; x < grid.getWidth(); ++x) {
                Cell& cell = grid.getCell(x, y);

                if (grid.getLiquidAmount(x, y, Material::Lava) <= 0.0f) {
                    cell.coolingTicks = 0;
                    continue;
                }

                int waterX = -1;
                int waterY = -1;

                if (grid.getLiquidAmount(x, y, Material::Water) > 0.001f) {
                    waterX = x;
                    waterY = y;
                }

                for (auto [dx, dy] : offsets) {
                    if (waterX != -1) {
                        break;
                    }

                    int nx = x + dx;
                    int ny = y + dy;

                    if (!grid.isValidCell(nx, ny)) {
                        continue;
                    }

                    if (grid.getLiquidAmount(nx, ny, Material::Water) > 0.001f) {
                        waterX = nx;
                        waterY = ny;
                    }
                }

                if (waterX == -1) {
                    cell.coolingTicks = 0;
                    continue;
                }

                if (cell.coolingTicks == 0) {
                    cell.coolingTicks = std::uniform_int_distribution<int>(1, 12)(rng);
                }

                ++cell.coolingTicks;

                if (cell.coolingTicks < 24) {
                    continue;
                }

                float waterAmount =
                    grid.getLiquidAmount(waterX, waterY, Material::Water);

                grid.removeLiquid(
                    waterX,
                    waterY,
                    Material::Water,
                    std::min(waterAmount, 0.25f)
                );

                grid.eraseCell(x, y);
                grid.placeSolid(x, y, Material::CoolingRock);

                spawnSteam(grid, x, y);
            }
        }
    }

    void Simulation::spawnSteam(Grid& grid, int x, int y) {
        std::array<std::pair<int, int>, 5> offsets{{
            {0, -1},
            {-1, -1},
            {1, -1},
            {-1, 0},
            {1, 0}
        }};

        std::shuffle(offsets.begin(), offsets.end(), rng);

        for (auto [dx, dy] : offsets) {
            int nx = x + dx;
            int ny = y + dy;

            if (!grid.isValidCell(nx, ny)) {
                continue;
            }

            Cell& target = grid.getCell(nx, ny);

            if (target.solid != Material::Empty ||
                target.gas != Material::Empty) {
                continue;
            }

            float totalLiquid = grid.getTotalLiquidAmount(nx, ny);
            float waterAmount = grid.getLiquidAmount(nx, ny, Material::Water);

            if (totalLiquid - waterAmount > 0.001f) {
                continue;
            }

            target.gas = Material::Steam;
            target.gasLifetime =
                std::uniform_int_distribution<int>(65, 110)(rng);
            target.lastGasUpdatedFrame = currentFrame;

            return;
        }
    }

    bool Simulation::canMoveSolid(Grid& grid, int x, int y, MoveDirection direction) const {
        if (!grid.isValidCell(x, y)) {
            return false;
        }

        Cell& source = grid.getCell(x, y);

        if (source.solid == Material::Empty) {
            return false;
        }
        
        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(targetX, targetY)) {
            return false;
        }

        Cell& target = grid.getCell(targetX, targetY);

        if (target.solid != Material::Empty) {
            return false;
        }

        if (target.liquids.empty()) {
            return true;
        }

        float solidDensity = getMaterialProperties(source.solid).density;
        for (const LiquidLayer& layer : target.liquids) {
            float liquidDensity = getMaterialProperties(layer.material).density;
            if (solidDensity <= liquidDensity) {
                return false;
            }
        }

        return true;
    }

    std::pair<int, int> Simulation::getTargetPosition(int x, int y, MoveDirection direction) const {
        int targetX = x, targetY = y;

        switch (direction)
        {
            case MoveDirection::Down:
                targetY++;
                break;
            case MoveDirection::DownLeft:
                targetX--;
                targetY++;
                break;
            case MoveDirection::DownRight:
                targetX++;
                targetY++;
                break;
            case MoveDirection::Left:
                targetX--;
                break;
            case MoveDirection::Right:
                targetX++;
                break;
            case MoveDirection::UpLeft:
                targetX--;
                targetY--;
                break;
            case MoveDirection::UpRight:
                targetX++;
                targetY--;
                break;
            case MoveDirection::Up:
                targetY--;
                break;
        }

        return {targetX, targetY};
    }
} // namespace sandbox