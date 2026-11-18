#pragma once
#include "game.hpp"
namespace bowling {
inline float alleyPosition(float mouseX) {
    return std::clamp(geometry::launchMin +
                          (mouseX - 240.f) / 800.f * (geometry::launchMax - geometry::launchMin),
                      geometry::launchMin, geometry::launchMax);
}
// Shared hit region excludes the top scores and side HUD, but includes the actual tube.
inline bool coursePointer(float x, float y) {
    return x > 185 && x < 1090 && y > 120 && y < 790;
}
inline float heldPower(float seconds) {
    return .15f + .85f * (.5f - .5f * std::cos(seconds * 4));
}
// Each confirmation advances exactly one step; only Power may launch.
enum class ShotPhase { Spot, Curve, Power };
struct StadiumSetup {
    ShotPhase phase = ShotPhase::Spot;
    float seconds = 0;
    void reset() {
        phase = ShotPhase::Spot;
        seconds = 0;
    }
    void back() {
        phase = phase == ShotPhase::Power ? ShotPhase::Curve : ShotPhase::Spot;
        seconds = 0;
    }
    void adjust(Shot &shot, float position, float curve) const {
        if (phase == ShotPhase::Spot)
            shot.position = std::clamp(position, geometry::launchMin, geometry::launchMax);
        if (phase == ShotPhase::Curve)
            shot.curve = std::clamp(curve, -1.f, 1.f);
        shot.aim = 0; // Stadium shots use placement and hook, never simultaneous drag aiming.
    }
    void tick(Shot &shot, float dt) {
        if (phase == ShotPhase::Power) {
            seconds += dt;
            shot.power = heldPower(seconds);
        }
    }
    bool confirm() {
        if (phase == ShotPhase::Spot) {
            phase = ShotPhase::Curve;
            return false;
        }
        if (phase == ShotPhase::Curve) {
            phase = ShotPhase::Power;
            seconds = 0;
            return false;
        }
        return true;
    }
};
// Pull the tube backward and to either side; release sends it opposite the pull.
inline Shot mascotPull(Shot shot, float dx, float downward) {
    shot.power = std::clamp(.15f + downward / 160.f, .15f, 1.f);
    shot.aim = std::clamp(-dx / 200.f, -1.f, 1.f);
    shot.curve = 0;
    return shot;
}
} // namespace bowling
