#include "sandbox_lab/PowderPhysics.hpp"
#include "sandbox_lab/LiquidPhysics.hpp"
#include "sandbox_lab/MaterialProperties.hpp"
#include <utility>

namespace sandbox {
    PowderPhysics::PowderPhysics(std::mt19937 &rng, std::uniform_int_distribution<int> &randomDirection, std::uint64_t& currentFrame,
                                 LiquidPhysics &liquidPhysics)
        : rng(rng), randomDirection(randomDirection), currentFrame(currentFrame), liquidPhysics(liquidPhysics) {}

    void PowderPhysics::updatePowder(Grid& grid, int x, int y) {
        Cell& cell = grid.getCell(x, y);

        bool coolingRock = cell.solid == Material::CoolingRock;

        constexpr float gravity = 0.08f;
        float maxFallSpeed = coolingRock ? 2.5f : 2.0f;

        if (coolingRock) {
            bool canFall = canMoveSolid(grid, x, y, MoveDirection::Down);

            bool canFallLeft =
                canMoveSolid(grid, x, y, MoveDirection::DownLeft) &&
                canMoveSolid(grid, x, y, MoveDirection::Left);

            bool canFallRight =
                canMoveSolid(grid, x, y, MoveDirection::DownRight) &&
                canMoveSolid(grid, x, y, MoveDirection::Right);

            if (!canFall && !canFallLeft && !canFallRight) {
                cell.solid = Material::Stone;
                cell.velocityY = 0.0f;
                cell.movementRemainder = 0.0f;
                return;
            }
        }

        cell.velocityY = std::min(
            cell.velocityY + gravity,
            maxFallSpeed
        );

        cell.movementRemainder += cell.velocityY;

        int steps = static_cast<int>(cell.movementRemainder);

        cell.movementRemainder -= static_cast<float>(steps);

        int currentX = x;
        int currentY = y;

        for (int i = 0; i < steps; ++i) {
            if (moveSolid(grid, currentX, currentY, MoveDirection::Down)) {
                ++currentY;
                continue;
            }

            bool canMoveLeft = canMoveSolid(grid, currentX, currentY, MoveDirection::DownLeft) && canMoveSolid(grid, currentX, currentY, MoveDirection::Left);
            bool canMoveRight = canMoveSolid(grid, currentX, currentY, MoveDirection::DownRight) && canMoveSolid(grid, currentX, currentY, MoveDirection::Right);
            bool moved = false;

            if (canMoveLeft && canMoveRight) {
                bool moveLeft = randomDirection(rng) == 0;

                if (moveLeft) {
                    moved = moveSolid(grid, currentX, currentY, MoveDirection::DownLeft);
                    if (moved) {
                        --currentX;
                    }
                } else {
                    moved = moveSolid(grid, currentX, currentY, MoveDirection::DownRight);
                    if (moved) {
                        ++currentX;
                    }
                }
            } else if (canMoveLeft) {
                moved = moveSolid(grid, currentX, currentY, MoveDirection::DownLeft);
                if (moved) {
                    --currentX;
                }
            } else if (canMoveRight) {
                moved = moveSolid(grid, currentX, currentY, MoveDirection::DownRight);
                if (moved) {
                    ++currentX;
                }
            }

            if (moved) {
                ++currentY;

                Cell& movedCell = grid.getCell(currentX, currentY);
                movedCell.velocityY = std::min(movedCell.velocityY, 0.8f);
            } else {
                Cell& stoppedCell = grid.getCell(currentX, currentY);
                stoppedCell.velocityY = 0.0f;
                stoppedCell.movementRemainder = 0.0f;

                if (coolingRock) {
                    stoppedCell.solid = Material::Stone;
                }
            }
            break;
        }
    }

    bool PowderPhysics::moveSolid(Grid &grid, int x, int y, MoveDirection direction) {
        if (!canMoveSolid(grid, x, y, direction)) {
            return false;
        }

        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        Cell &source = grid.getCell(x, y);
        Cell &target = grid.getCell(targetX, targetY);

        if (!target.liquids.empty()) {
            liquidPhysics.displaceLiquidSideways(grid, targetX, targetY);
        }

        source.lastUpdatedFrame = currentFrame;
        std::swap(source, target);
        return true;
    }

    bool PowderPhysics::canMoveSolid(Grid &grid, int x, int y, MoveDirection direction) const {
        if (!grid.isValidCell(x, y)) {
            return false;
        }

        Cell &source = grid.getCell(x, y);

        if (source.solid == Material::Empty) {
            return false;
        }

        auto [targetX, targetY] = getTargetPosition(x, y, direction);

        if (!grid.isValidCell(targetX, targetY)) {
            return false;
        }

        Cell &target = grid.getCell(targetX, targetY);

        if (target.solid != Material::Empty) {
            return false;
        }

        if (target.liquids.empty()) {
            return true;
        }

        float solidDensity = getMaterialProperties(source.solid).density;
        for (const LiquidLayer &layer : target.liquids) {
            float liquidDensity = getMaterialProperties(layer.material).density;
            if (solidDensity <= liquidDensity) {
                return false;
            }
        }

        return true;
    }
}
