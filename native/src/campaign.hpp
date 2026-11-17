#pragma once
#include "game.hpp"
#include "teams.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
namespace bowling {
struct Arena {
    const char *name;
    const char *theme;
    int team;
};
inline constexpr Arena arenas[] = {
    {"Wrigley Field", "Ivy League", 4},         {"Fenway Park", "The Green Monster", 3},
    {"Oracle Park", "Splash Landing", 23},      {"PNC Park", "Steel City Showdown", 21},
    {"Coors Field", "Rocky Mountain Roll", 8},  {"Kauffman Stadium", "Fountain Frenzy", 11},
    {"Chase Field", "Desert Heat", 0},          {"T-Mobile Park", "Emerald Express", 24},
    {"Yankee Stadium", "Championship Row", 18}, {"Tropicana Field", "Rays of Glory", 26}};
inline std::string ballName(int id) {
    const char *fun[] = {
        "Desert Fangs",     "Peach Power",  "Birdland",    "Monster Mash",     "Ivy Cub",
        "Southside Sparks", "Red Machine",  "Lake Effect", "Purple Peaks",     "Motor City",
        "Orbit Blast",      "Crown Jewels", "Halo Glow",   "Blue Heaven",      "Splash Party",
        "Barrel Roll",      "Twin Cities",  "Big Apple",   "Pinstripe Legend", "Green & Gold",
        "Liberty Bell",     "Cannonball",   "Friar Fire",  "Bay Splash",       "Sea Monster",
        "Redbird Rally",    "Ray Wave",     "Lone Star",   "Northern Lights",  "Capital Fireworks"};
    const char *rays[] = {"Rays Sunburst", "Devil Ray", "Touch Tank", "Tropicana Finale"};
    if (id >= 60)
        return rays[id - 60];
    if (id >= 30)
        return std::string(teams[id - 30].id) + " - " + fun[id - 30];
    return teams[id].name;
}
struct Profile {
    int xp = 0, wins = 0, matches = 0, best = 0;
    unsigned cleared = 0;
    int balls() const {
        return std::min(64, 1 + xp / 80);
    }
    int stadiums() const {
        int n = 1;
        while (n < 10 && (cleared & (1u << (n - 1))))
            ++n;
        return n;
    }
    bool load(const std::string &path) {
        std::ifstream f(path);
        std::string magic;
        int version;
        Profile p;
        if (!(f >> magic >> version >> p.xp >> p.wins >> p.matches >> p.best >> p.cleared) ||
            magic != "BLB" || version != 1 || p.xp < 0 || p.xp > 100000000 || p.wins < 0 ||
            p.matches < p.wins || p.best < 0 || p.best > 300 || p.cleared > 1023)
            return false;
        *this = p;
        return true;
    }
    bool save(const std::string &path) const {
        auto temp = path + ".tmp";
        {
            std::ofstream f(temp);
            if (!(f << "BLB 1 " << xp << ' ' << wins << ' ' << matches << ' ' << best << ' ' << cleared
                    << '\n'))
                return false;
            f.flush();
            if (!f)
                return false;
        }
        std::error_code ec;
        auto backup = path + ".bak";
        if (std::filesystem::exists(path)) {
            std::filesystem::copy_file(path, backup, std::filesystem::copy_options::overwrite_existing,
                                       ec);
            if (ec)
                return false;
            std::filesystem::remove(path, ec);
            if (ec)
                return false;
        }
        std::filesystem::rename(temp, path, ec);
        if (ec) {
            std::error_code ignored;
            if (std::filesystem::exists(backup))
                std::filesystem::copy_file(backup, path,
                                           std::filesystem::copy_options::overwrite_existing, ignored);
            return false;
        }
        return true;
    }
    int award(int score, int opponent, int arena) {
        ++matches;
        best = std::max(best, score);
        bool win = score > opponent;
        if (win) {
            ++wins;
            cleared |= 1u << arena;
        }
        int gained = 120 + score + (win ? 150 : 0);
        xp = std::min(100000000, xp + gained);
        return gained;
    }
};
enum class Power { None, Heavy, Magnet, Bumpers };
struct Match {
    std::array<Game, 2> games{};
    std::array<Simulation, 2> lanes{};
    int player = 0, stage = 0;
    bool mascot = false;
    std::array<std::array<int, 3>, 2> stock{{{2, 2, 2}, {2, 2, 2}}};
    Power equipped = Power::None;
    bool boostUsed = false;
    float steering = 0;
    std::mt19937 rng{981};
    bool complete() const {
        return games[0].complete && games[1].complete;
    }
    bool equip(Power p) {
        int i = int(p) - 1;
        if (i < 0 || i > 2 || lanes[player].active || stock[player][i] == 0)
            return false;
        equipped = equipped == p ? Power::None : p;
        return true;
    }
    void launch(Shot s) {
        auto &sim = lanes[player];
        if (!sim.launch(s))
            return;
        boostUsed = false;
        if (equipped != Power::None) {
            --stock[player][int(equipped) - 1];
            if (equipped == Power::Heavy)
                sim.ball.r = .85f;
        }
    }
    bool step(float dt) {
        auto &s = lanes[player];
        if (s.active) {
            if (mascot && !s.gutter && s.ball.z < 22)
                s.ball.vx = std::clamp(s.ball.vx + steering * 12 * dt, -7.f, 7.f);
            if (equipped == Power::Magnet && !s.gutter && s.ball.z > 12 && !s.pins.empty()) {
                auto p = std::min_element(s.pins.begin(), s.pins.end(), [&](auto &a, auto &b) {
                    return std::abs(a.x - s.ball.x) < std::abs(b.x - s.ball.x);
                });
                s.ball.vx += (p->x - s.ball.x) * 2 * dt;
            }
            if (equipped == Power::Bumpers) {
                float edge = s.ball.r + .07f;
                if (s.ball.x < edge) {
                    s.ball.x = edge;
                    s.ball.vx = std::abs(s.ball.vx) * .7f;
                    s.gutter = false;
                }
                if (s.ball.x > 10 - edge) {
                    s.ball.x = 10 - edge;
                    s.ball.vx = -std::abs(s.ball.vx) * .7f;
                    s.gutter = false;
                }
            }
        }
        return s.step(dt);
    }
    void boost() {
        auto &s = lanes[player];
        if (mascot && s.active && !boostUsed) {
            s.ball.vz *= 1.25f;
            boostUsed = true;
        }
    }
    void settle() {
        auto &g = games[player];
        auto &s = lanes[player];
        int old = g.frame;
        g.roll(s.knocked());
        if (g.freshRack())
            s.resetRack();
        else
            s.clearFallen();
        s.hasBall = false;
        equipped = Power::None;
        steering = 0;
        if (g.complete || g.frame != old) {
            int other = 1 - player;
            if (!games[other].complete)
                player = other;
        }
    }
    Shot aiShot() {
        auto &s = lanes[1];
        float target = 5;
        if (!s.pins.empty()) {
            target = 0;
            for (auto &p : s.pins)
                target += p.x;
            target /= s.pins.size();
        }
        std::uniform_real_distribution<float> error(-1, 1);
        float spread = .75f - stage * .055f;
        return {std::clamp(target + error(rng) * spread, .7f, 9.3f), error(rng) * spread,
                .55f + .03f * stage, error(rng) * .25f};
    }
};
} // namespace bowling
