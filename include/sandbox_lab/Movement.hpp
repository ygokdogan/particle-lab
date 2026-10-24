#pragma once

#include <utility>

namespace sandbox {
    enum class MoveDirection { Down, DownLeft, DownRight, Left, Right, UpLeft, UpRight, Up };

    inline std::pair<int, int> getTargetPosition(int x, int y, MoveDirection direction) {
        int targetX = x, targetY = y;

        switch (direction) {
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
}
