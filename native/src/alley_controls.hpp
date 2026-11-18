#pragma once
#include "game.hpp"
namespace bowling {
inline float alleyPosition(float mouseX) {
    return std::clamp(.55f + (mouseX - 240.f) / 800.f * 8.9f, .55f, 9.45f);
}
inline Shot alleyDrag(Shot shot, float dx, float upward) {
    shot.power = std::clamp(.15f + upward / 180.f, .15f, 1.f);
    shot.aim = std::clamp(dx / 240.f, -1.f, 1.f);
    return shot;
}
} // namespace bowling
