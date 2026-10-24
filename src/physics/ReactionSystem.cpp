#include "sandbox_lab/ReactionSystem.hpp"
#include "sandbox_lab/GasPhysics.hpp"
#include <algorithm>
#include <array>
#include <utility>

namespace sandbox {
    ReactionSystem::ReactionSystem(std::mt19937 &rng, GasPhysics &gasPhysics) : rng(rng), gasPhysics(gasPhysics) {}

    void ReactionSystem::updateWood(Grid &grid, int x, int y) {
        constexpr std::array<std::pair<int, int>, 8> neighbors{{{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}};

        Cell &cell = grid.getCell(x, y);

        if (cell.burnTicks == 0) {
            std::bernoulli_distribution ignitionChance(0.02);

            for (auto [dx, dy] : neighbors) {
                int nx = x + dx;
                int ny = y + dy;

                if (!grid.isValidCell(nx, ny)) {
                    continue;
                }

                if (grid.getLiquidAmount(nx, ny, Material::Lava) >= 0.25f && ignitionChance(rng)) {
                    igniteWood(grid, x, y);
                    return;
                }
            }
            return;
        }

        if (std::bernoulli_distribution(0.08)(rng)) {
            gasPhysics.spawnSmoke(grid, x, y);
        }

        std::bernoulli_distribution spreadChance(0.012);

        for (auto [dx, dy] : neighbors) {
            int nx = x + dx;
            int ny = y + dy;

            if (!grid.isValidCell(nx, ny)) {
                continue;
            }

            const Cell &neighbor = grid.getCell(nx, ny);

            if (neighbor.solid == Material::Wood && neighbor.burnTicks == 0 && spreadChance(rng)) {
                igniteWood(grid, nx, ny);
            }
        }

        ++cell.burnTicks;

        if (cell.burnTicks >= cell.burnDuration) {
            grid.eraseCell(x, y);
        }
    }

    void ReactionSystem::updateLava(Grid &grid, int x, int y) {
        float lavaAmount = grid.getLiquidAmount(x, y, Material::Lava);

        if (lavaAmount <= 0.25f) {
            return;
        }

        std::array<std::pair<int, int>, 4> offsets{{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

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

            Cell &neighbor = grid.getCell(nx, ny);

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

    void ReactionSystem::igniteWood(Grid &grid, int x, int y) {
        if (!grid.isValidCell(x, y)) {
            return;
        }

        Cell &cell = grid.getCell(x, y);

        if (cell.solid != Material::Wood || cell.burnTicks != 0) {
            return;
        }

        cell.burnTicks = 1;
        cell.burnDuration = std::uniform_int_distribution<int>(100, 240)(rng);
    }

    void ReactionSystem::coolLava(Grid &grid) {
        constexpr std::array<std::pair<int, int>, 4> offsets{{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

        for (int y = 0; y < grid.getHeight(); ++y) {
            for (int x = 0; x < grid.getWidth(); ++x) {
                Cell &cell = grid.getCell(x, y);

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

                float waterAmount = grid.getLiquidAmount(waterX, waterY, Material::Water);

                grid.removeLiquid(waterX, waterY, Material::Water, std::min(waterAmount, 0.25f));

                grid.eraseCell(x, y);
                grid.placeSolid(x, y, Material::CoolingRock);

                gasPhysics.spawnSteam(grid, x, y);
            }
        }
    }
}
