#include "game.hpp"
#include <cstdlib>
#include <iostream>
using namespace bowling;
void check(bool b, const char *message) {
    if (!b) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}
int main() {
    Game perfect;
    for (int i = 0; i < 12; ++i)
        perfect.roll(10);
    check(perfect.complete && perfect.scores()[9] == 300, "perfect game");
    Game spare;
    for (int i = 0; i < 21; ++i)
        spare.roll(5);
    check(spare.complete && spare.scores()[9] == 150, "spare bonuses");
    Game gutter;
    for (int i = 0; i < 20; ++i)
        gutter.roll(0);
    check(gutter.complete && gutter.scores()[9] == 0, "gutter game");
    Game bonus;
    bonus.roll(10);
    bonus.roll(10);
    check(!bonus.scores()[0], "pending bonus");
    bonus.roll(4);
    check(bonus.scores()[0] == 24, "resolved bonus");
    Game tenth;
    for (int i = 0; i < 18; ++i)
        tenth.roll(0);
    tenth.roll(10);
    tenth.roll(7);
    check(tenth.standing() == 3 && !tenth.freshRack(), "tenth rack restriction");
    bool rejected = false;
    try {
        tenth.roll(4);
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    check(rejected && tenth.frames[9].size() == 2, "invalid roll preserves state");
    tenth.roll(3);
    check(tenth.complete && tenth.scores()[9] == 20, "tenth bonus score");
    check(geometry::width == 10 && geometry::headPin - geometry::foulLine == 80,
          "fixed arcade lane proportion baseline");
    check(geometry::ballRadius == .85f && geometry::pinRadius == .55f && geometry::pinSpacingX == 2.65f,
          "fixed ball and rack proportion baseline");
    check(geometry::stadiumCamera.fov == 55 && geometry::mascotCamera.fov == 53,
          "art updates preserve camera framing presets");
    check(geometry::stadiumCamera.height == 6.5f && geometry::stadiumCamera.back == -12.f &&
              geometry::stadiumCamera.targetHeight == .6f && geometry::mascotCamera.height == 4.f &&
              geometry::mascotCamera.back == -7.f && geometry::mascotCamera.targetHeight == -1.f,
          "full camera baseline stays fixed across stadium artwork updates");
    // Independent perspective projection: guard visible distance, not just world length.
    auto projectedY = [](float z) {
        const auto c = geometry::stadiumCamera;
        float dz = geometry::headPin - c.back, dy = c.targetHeight - c.height;
        float length = std::sqrt(dz * dz + dy * dy), fy = dy / length, fz = dz / length;
        float depth = -c.height * fy + (z - c.back) * fz;
        float up = -c.height * fz - (z - c.back) * fy;
        return 400.f - 400.f * up / (depth * std::tan(c.fov * 3.14159265f / 360.f));
    };
    float visibleRun = projectedY(geometry::foulLine) - projectedY(geometry::headPin);
    check(visibleRun > 285 && visibleRun < 305, "launch view preserves lane run on an 800px display");
    check(geometry::renderX(geometry::launchMin) > geometry::renderX(geometry::launchMax),
          "mouse right maps to camera right looking down positive Z");
    Simulation dimensions;
    for (const auto &pin : dimensions.pins)
        check(pin.x - pin.r >= 0 && pin.x + pin.r <= 10 && pin.z < geometry::deckEnd,
              "full rack fits playable deck");
    dimensions.launch({0, 0, .7f, 0});
    check(dimensions.ball.x >= dimensions.ball.r, "launch keeps ball inside lane");
    Simulation a, b;
    Shot shot;
    a.launch(shot);
    b.launch(shot);
    for (int i = 0; i < 1000; ++i) {
        a.step(1.f / 120);
        b.step(1.f / 120);
    }
    check(!a.active && a.knocked() > 0 && a.knocked() == b.knocked(), "deterministic collisions");
    Simulation miss;
    miss.launch({1, -1, .7f, 0});
    for (int i = 0; i < 1000; ++i)
        miss.step(1.f / 120);
    check(miss.knocked() == 0 && miss.gutter, "gutter misses rack");
    Game full;
    Simulation sim;
    int rolls = 0;
    while (!full.complete && rolls++ < 22) {
        check(int(sim.pins.size()) == full.standing(), "rack matches score");
        sim.launch({5, .08f, .75f, -.1f});
        for (int i = 0; i < 1000 && sim.active; ++i)
            sim.step(1.f / 120);
        check(!sim.active, "shot settles");
        full.roll(sim.knocked());
        if (full.freshRack())
            sim.resetRack();
        else
            sim.clearFallen();
    }
    check(full.complete && *full.scores()[9] >= 0 && *full.scores()[9] <= 300,
          "complete simulated match");
    std::cout << "Native scoring, geometry regression and physics checks passed\n";
}
