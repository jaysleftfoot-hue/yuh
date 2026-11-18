#pragma once
#include "game.hpp"
namespace bowling {
inline float alleyPosition(float mouseX) {
    return std::clamp(geometry::launchMin +
                          (mouseX - 240.f) / 800.f * (geometry::launchMax - geometry::launchMin),
                      geometry::launchMin, geometry::launchMax);
}
inline Shot alleyDrag(Shot shot, float dx, float upward) {
    shot.power = std::clamp(.15f + upward / 180.f, .15f, 1.f);
    shot.aim = std::clamp(dx / 240.f, -1.f, 1.f);
    return shot;
}
// Pull the tube backward and to either side; release sends it opposite the pull.
inline Shot mascotPull(Shot shot, float dx, float downward) {
    shot.power = std::clamp(.15f + downward / 160.f, .15f, 1.f);
    shot.aim = std::clamp(-dx / 200.f, -1.f, 1.f);
    shot.curve = 0;
    return shot;
}
} // namespace bowling
