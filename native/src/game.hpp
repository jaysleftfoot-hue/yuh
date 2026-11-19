#pragma once
#include "lane_geometry.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
namespace bowling {
struct Game {
    std::array<std::vector<int>, 10> frames{};
    int frame = 0;
    bool complete = false;
    // Both the scorekeeper and the lane need to know when a new rack starts.
    bool startsRack(int f, size_t ball) const {
        const auto &r = frames[f];
        if (ball == 0)
            return true;
        if (f != 9)
            return false;
        if (ball == 1)
            return r[0] == 10;
        return ball == 2 && (r[0] == 10 ? r[1] == 10 : r[0] + r[1] == 10);
    }
    bool freshRack() const {
        return startsRack(frame, frames[frame].size());
    }
    std::string marks(int f) const {
        const auto &r = frames[f];
        std::string result;
        for (size_t i = 0; i < r.size(); ++i) {
            if (i)
                result += ' ';
            if (!startsRack(f, i) && r[i - 1] + r[i] == 10)
                result += '/';
            else if (r[i] == 10)
                result += 'X';
            else
                result += r[i] ? std::to_string(r[i]) : "-";
        }
        return result;
    }
    int standing() const {
        if (complete)
            return 0;
        if (freshRack())
            return 10;
        const auto &r = frames[frame];
        int n = 0;
        for (int v : r)
            n += v;
        return 10 - (frame == 9 && r[0] == 10 ? r[1] : n);
    }
    void roll(int pins) {
        if (complete || pins < 0 || pins > standing())
            throw std::invalid_argument("Invalid roll");
        auto &r = frames[frame];
        r.push_back(pins);
        if (frame < 9) {
            if (pins == 10 || r.size() == 2)
                ++frame;
        } else if (r.size() == 3 || (r.size() == 2 && r[0] + r[1] < 10))
            complete = true;
    }
    std::array<std::optional<int>, 10> scores() const {
        std::array<std::optional<int>, 10> out{};
        int total = 0;
        for (int i = 0; i < 10; ++i) {
            const auto &r = frames[i];
            if (i == 9) {
                if (complete) {
                    for (int v : r)
                        total += v;
                    out[i] = total;
                }
                break;
            }
            std::vector<int> next;
            for (int j = i + 1; j < 10; ++j)
                for (int v : frames[j])
                    next.push_back(v);
            if (r.empty())
                break;
            if (r[0] == 10) {
                if (next.size() < 2)
                    break;
                total += 10 + next[0] + next[1];
            } else if (r.size() == 2) {
                if (r[0] + r[1] == 10) {
                    if (next.empty())
                        break;
                    total += 10 + next[0];
                } else
                    total += r[0] + r[1];
            } else
                break;
            out[i] = total;
        }
        return out;
    }
};
struct Body {
    float x = 0, z = 0, vx = 0, vz = 0, r = geometry::pinRadius;
    bool down = false;
    int id = 0;
    float fall = 0;
};
struct Shot {
    float position = 5, aim = 0, power = .72f, curve = 0;
};
struct Simulation {
    geometry::LaneLayout layout;
    std::vector<Body> pins;
    Body ball{};
    bool active = false, hasBall = false, gutter = false;
    float elapsed = 0, curve = 0;
    int impacts = 0, bankedPin = -1;
    explicit Simulation(geometry::LaneLayout dimensions = geometry::stadiumLane) : layout(dimensions) {
        resetRack();
    }
    void resetRack() {
        pins.clear();
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col <= row; ++col)
                pins.push_back({geometry::rackX(row, col), geometry::rackZ(row, layout), 0, 0,
                                geometry::pinRadius, false, int(pins.size()), 0});
    }
    void clearFallen() {
        pins.erase(std::remove_if(pins.begin(), pins.end(), [](const Body &p) { return p.down; }),
                   pins.end());
    }
    bool launch(Shot s) {
        if (active)
            return false;
        float speed = geometry::launchSpeed(std::clamp(s.power, 0.f, 1.f)),
              angle = std::clamp(s.aim, -1.f, 1.f) * .16f;
        ball = {std::clamp(s.position, geometry::launchMin, geometry::launchMax), geometry::foulLine,
                std::sin(angle) * speed, std::cos(angle) * speed, geometry::ballRadius};
        curve = std::clamp(s.curve, -1.f, 1.f);
        active = hasBall = true;
        gutter = false;
        elapsed = 0;
        impacts = 0;
        bankedPin = -1;
        return true;
    }
    void collide(Body &a, Body &b, bool isBall) {
        float dx = b.x - a.x, dz = b.z - a.z, d = std::hypot(dx, dz), r = a.r + b.r;
        if (d >= r || d < .00001f)
            return;
        float nx = dx / d, nz = dz / d, relative = (a.vx - b.vx) * nx + (a.vz - b.vz) * nz;
        if (relative > .05f) {
            float impulse = relative * (isBall ? 1.3f : .8f);
            b.vx += nx * impulse;
            b.vz += nz * impulse;
            a.vx -= nx * impulse * (isBall ? .12f : 1.f);
            a.vz -= nz * impulse * (isBall ? .12f : 1.f);
            if (impulse > .65f) {
                if (!b.down)
                    ++impacts;
                b.down = true;
                if (!isBall)
                    a.down = true;
            }
        }
        b.x += nx * (r - d + .001f);
        b.z += nz * (r - d + .001f);
    }
    bool step(float dt) {
        for (auto &p : pins)
            if (p.down)
                p.fall = std::min(1.f, p.fall + dt * 4);
        if (!active)
            return false;
        elapsed += dt;
        if (ball.z < layout.deckEnd()) {
            if (!gutter)
                ball.vx += curve * .85f * dt;
            ball.x += ball.vx * dt;
            ball.z += ball.vz * dt;
            if (ball.x < ball.r || ball.x > geometry::width - ball.r) {
                gutter = true;
                ball.x = std::clamp(ball.x, .1f, geometry::width - .1f);
                ball.vx = 0;
            }
        }
        for (auto &p : pins) {
            p.x += p.vx * dt;
            p.z += p.vz * dt;
            float drag = std::exp(-2.4f * dt);
            p.vx *= drag;
            p.vz *= drag;
            if (p.x < p.r || p.x > geometry::width - p.r) {
                p.x = std::clamp(p.x, p.r, geometry::width - p.r);
                p.vx *= -.3f;
            }
            if (!gutter && ball.z < layout.deckEnd() && p.id != bankedPin) {
                const bool wasStanding = !p.down;
                const float contactOffset = ball.x - p.x;
                collide(ball, p, true);
                if (wasStanding && p.down && pins.size() == 2 && pins[0].id == 6 && pins[1].id == 9) {
                    Body &opposite = p.id == 6 ? pins[1] : pins[0];
                    const float inward = opposite.x > p.x ? 1.f : -1.f;
                    const float graze = contactOffset * inward;
                    // A firm inside-edge hit can bank a corner pin across the deck in the 7–10.
                    if (!opposite.down && graze > .35f && graze < 1.15f && ball.vz > 35.f) {
                        bankedPin = p.id;
                        p.vx = inward * 28.f;
                        p.vz = (opposite.z - p.z) * 3.f;
                    }
                }
            }
        }
        for (size_t i = 0; i < pins.size(); ++i)
            for (size_t j = i + 1; j < pins.size(); ++j)
                collide(pins[i], pins[j], false);
        bool moving = false;
        for (const auto &p : pins)
            if (std::hypot(p.vx, p.vz) > .08f)
                moving = true;
        if (elapsed > geometry::shotTimeout || (ball.z > layout.deckEnd() - 1 && !moving)) {
            active = false;
            return true;
        }
        return false;
    }
    std::array<bool, 10> standingPins() const {
        std::array<bool, 10> standing{};
        for (const auto &pin : pins)
            standing[pin.id] = !pin.down;
        return standing;
    }
    int standingCount() const {
        return int(pins.size()) - knocked();
    }
    int knocked() const {
        return int(std::count_if(pins.begin(), pins.end(), [](const Body &p) { return p.down; }));
    }
};
} // namespace bowling
