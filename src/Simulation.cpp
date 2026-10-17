#include <algorithm>
#include <utility>
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
                if (cell.lastUpdatedFrame == currentFrame){
                    continue;
                }

                const MaterialProperties& properties = getMaterialProperties(cell.material);

                switch (properties.behaviour) {
                    case Behaviour::Powder:
                        updatePowder(grid, x, y);
                        break;
                    case Behaviour::Liquid:
                        updateLiquid(grid, x, y);
                        break;
                    default:
                        break;
                }
            }
        }

        scanLeftToRight = !scanLeftToRight;
    }

    void Simulation::updatePowder(Grid& grid, int x, int y) {
        if (moveCell(grid, x, y, MoveDirection::Down)) {
            return;
        }

        bool canMoveLeft = grid.isEmptyCell(x-1, y+1);
        bool canMoveRight = grid.isEmptyCell(x+1, y+1);

        if (canMoveLeft && canMoveRight) {
            bool moveLeft = randomDirection(rng) == 0;

            if (moveLeft) {
                moveCell(grid, x, y, MoveDirection::DownLeft);
            }
            else {
                moveCell(grid, x, y, MoveDirection::DownRight);
            }
            return;
        }

        if (canMoveRight) {
            moveCell(grid, x, y, MoveDirection::DownRight);
            return;
        }

        if (canMoveLeft) {
            moveCell(grid, x, y, MoveDirection::DownLeft);
            return;
        }
    }

    void Simulation::updateLiquid(Grid& grid, int x, int y) {
        Cell& cell = grid.getCell(x, y);

        if (moveCell(grid, x, y, MoveDirection::Down)) {
            return;
        }

        bool canMoveDownLeft = grid.isEmptyCell(x-1, y+1) || canFlowTowardsLowerLevel(grid, x, y, MoveDirection::Left);
        bool canMoveDownRight = grid.isEmptyCell(x+1, y+1) || canFlowTowardsLowerLevel(grid, x, y, MoveDirection::Right);

        if (canMoveDownLeft && canMoveDownRight) {
            bool moveLeft = randomDirection(rng) == 0;

            if (moveLeft) {
                moveCell(grid, x, y, MoveDirection::DownLeft);
            }
            else {
                moveCell(grid, x, y, MoveDirection::DownRight);
            }
            return;
        }

        if (canMoveDownRight) {
            moveCell(grid, x, y, MoveDirection::DownRight);
            return;
        }

        if (canMoveDownLeft) {
            moveCell(grid, x, y, MoveDirection::DownLeft);
            return;
        }

        bool canMoveRight = grid.isEmptyCell(x+1, y);
        bool canMoveLeft = grid.isEmptyCell(x-1, y);

        if (canMoveRight && canMoveLeft) {
            bool moveLeft = randomDirection(rng) == 0;

            if (moveLeft) {
                moveCell(grid, x, y, MoveDirection::Left);
            }
            else {
                moveCell(grid, x, y, MoveDirection::Right);
            }
            return;
        }

        if (canMoveRight) {
            moveCell(grid, x, y, MoveDirection::Right);
            return;
        }

        if (canMoveLeft) {
            moveCell(grid, x, y, MoveDirection::Left);
            return;
        }
    }

    bool Simulation::canFlowTowardsLowerLevel(const Grid& grid, int x, int y, MoveDirection direction) {
        int step = direction == MoveDirection::Right ? 1 : -1;

        for (int dist = 1; dist <= maxFlowDistance; dist++) {
            int checkX = x + dist * step;
            if (!grid.isEmptyCell(checkX, y)) {
                return false;
            }

            if (grid.isEmptyCell(checkX, y+1)) {
                return true;
            }
        }

        return false;
    }

    bool Simulation::moveCell(Grid& grid, int x, int y, MoveDirection direction) {
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
        
        if(!grid.isEmptyCell(targetX, targetY)) {
            return false;
        }

        grid.getCell(x, y).lastUpdatedFrame = currentFrame;
        std::swap(
            grid.getCell(x, y), 
            grid.getCell(targetX, targetY)
        );
        return true;
    }
} // namespace sandbox