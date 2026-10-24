#include "sandbox_lab/GasPhysics.hpp"
#include <algorithm>
#include <array>

namespace sandbox {
    GasPhysics::GasPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection, std::uint64_t currentFrame)
        : rng(rng), randomDirection(randomDirection), currentFrame(currentFrame) {}

    void GasPhysics::updateGas(Grid &grid, int x, int y) {
        Cell &cell = grid.getCell(x, y);

        if (cell.gas == Material::Empty || cell.lastGasUpdatedFrame == currentFrame) {
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
                leftFirst ? MoveDirection::Right : MoveDirection::Left};

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

    void GasPhysics::spawnSmoke(Grid &grid, int x, int y) {
        std::array<std::pair<int, int>, 5> offsets{
            {{0, -1}, {-1, -1}, {1, -1}, {-1, 0}, {1, 0}}};

        std::shuffle(offsets.begin(), offsets.end(), rng);

        for (auto [dx, dy] : offsets) {
            int targetX = x + dx;
            int targetY = y + dy;

            if (!grid.isValidCell(targetX, targetY)) {
            continue;
            }

            Cell &target = grid.getCell(targetX, targetY);

            if (target.solid != Material::Empty || !target.liquids.empty() ||
                target.gas != Material::Empty) {
            continue;
            }

            target.gas = Material::Smoke;
            target.gasLifetime = std::uniform_int_distribution<int>(240, 350)(rng);
            target.lastGasUpdatedFrame = currentFrame;

            return;
        }
    }

    bool GasPhysics::moveGas(Grid &grid, int x, int y, MoveDirection direction) {
        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(x, y) || !grid.isValidCell(targetX, targetY)) {
            return false;
        }

        Cell &source = grid.getCell(x, y);
        Cell &target = grid.getCell(targetX, targetY);

        if (source.gas == Material::Empty || target.gas != Material::Empty ||
            target.solid != Material::Empty) {
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

    void GasPhysics::spawnSteam(Grid &grid, int x, int y) {
        std::array<std::pair<int, int>, 5> offsets{
            {{0, -1}, {-1, -1}, {1, -1}, {-1, 0}, {1, 0}}};

        std::shuffle(offsets.begin(), offsets.end(), rng);

        for (auto [dx, dy] : offsets) {
            int nx = x + dx;
            int ny = y + dy;

            if (!grid.isValidCell(nx, ny)) {
            continue;
            }

            Cell &target = grid.getCell(nx, ny);

            if (target.solid != Material::Empty || target.gas != Material::Empty) {
            continue;
            }

            float totalLiquid = grid.getTotalLiquidAmount(nx, ny);
            float waterAmount = grid.getLiquidAmount(nx, ny, Material::Water);

            if (totalLiquid - waterAmount > 0.001f) {
            continue;
            }

            target.gas = Material::Steam;
            target.gasLifetime = std::uniform_int_distribution<int>(65, 110)(rng);
            target.lastGasUpdatedFrame = currentFrame;

            return;
        }
    }
}
