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
        "Sonoran Sidewinder", "Peachtree Feather", "Camden Oriole", "The Green Monster", "Ivy League Cub",
        "South Side Pinwheel", "Riverboat Machine", "Guardian of Traffic", "Purple Peaks", "Motor City Prowler",
        "Rainbow Express", "Crown Fountain", "Golden Halo", "Chavez Ravine", "Teal Marlin",
        "Barrelman", "Twin Cities Crossing", "Home Run Apple", "Pinstripe Legend", "White Elephant",
        "Liberty Bell", "Three Rivers Fleet", "Swinging Friar", "Cove Splash", "Emerald Trident",
        "Birds on the Bat", "Gulf Breaker", "Lone Star Silver", "Northern Flight", "Capitol Fireworks"};
    const char *rays[] = {"Rays Sunburst", "Devil Ray", "Touch Tank", "Tropicana Finale"};
    if (id >= 60)
        return rays[id - 60];
    if (id >= 30)
        return std::string(teams[id - 30].id) + " - " + fun[id - 30];
    return teams[id].name;
}
enum class Challenge { WrigleyWin, FenwaySplit, MascotPickups };
struct ChallengeReward {
    Challenge challenge;
    int ball;
    const char *instruction;
};
inline constexpr ChallengeReward challenges[] = {
    {Challenge::WrigleyWin, 34, "BEAT THE RIVAL AT WRIGLEY"},
    {Challenge::FenwaySplit, 33, "CONVERT THE 7-10 SPLIT AT FENWAY"},
    {Challenge::MascotPickups, 62, "COLLECT THREE TOKENS IN ONE MASCOT RIDE"},
};
inline constexpr unsigned challengeBit(Challenge challenge) {
    return 1u << int(challenge);
}
inline const ChallengeReward *ballChallenge(int ball) {
    for (const auto &reward : challenges)
        if (reward.ball == ball)
            return &reward;
    return nullptr;
}

struct Profile {
    int xp = 0, wins = 0, matches = 0, best = 0;
    unsigned cleared = 0, completedChallenges = 0;
    bool completed(Challenge challenge) const {
        return (completedChallenges & challengeBit(challenge)) != 0;
    }
    bool finish(Challenge challenge) {
        if (completed(challenge))
            return false;
        completedChallenges |= challengeBit(challenge);
        return true;
    }
    bool ballUnlocked(int ball) const {
        const auto *reward = ballChallenge(ball);
        return ball >= 0 && ball < 64 &&
               (xp >= ball * 80 || (reward && completed(reward->challenge)));
    }
    int balls() const {
        int count = 0;
        for (int ball = 0; ball < 64; ++ball)
            count += ballUnlocked(ball);
        return count;
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
            magic != "BLB" || (version != 1 && version != 2) || p.xp < 0 || p.xp > 100000000 || p.wins < 0 ||
            p.matches < p.wins || p.best < 0 || p.best > 300 || p.cleared > 1023)
            return false;
        if (version == 2 && (!(f >> p.completedChallenges) || p.completedChallenges > 7))
            return false;
        *this = p;
        return true;
    }
    bool save(const std::string &path) const {
        auto temp = path + ".tmp";
        {
            std::ofstream f(temp);
            if (!(f << "BLB 2 " << xp << ' ' << wins << ' ' << matches << ' ' << best << ' ' << cleared
                    << ' ' << completedChallenges << '\n'))
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
enum class Power { None, Heavy, Magnet, Bumpers, Fastball, Changeup, WildPitch };
inline constexpr int powerCount = int(Power::WildPitch);
inline constexpr int stockLimit = 3;
inline bool isTrick(Power power) {
    return power == Power::Changeup || power == Power::WildPitch;
}
inline const char *powerName(Power power) {
    constexpr const char *names[] = {"", "HEAVY", "MAGNET", "BUMPERS", "FASTBALL", "CHANGEUP", "WILD PITCH"};
    return names[int(power)];
}
enum class RivalDifficulty { Rookie, Pro, AllStar };
inline const char *difficultyName(RivalDifficulty difficulty) {
    switch (difficulty) {
    case RivalDifficulty::Rookie:
        return "ROOKIE";
    case RivalDifficulty::Pro:
        return "PRO";
    case RivalDifficulty::AllStar:
        return "ALL-STAR";
    }
    return "PRO";
}
enum class RivalStyle { Slugger, Umpire, BallBoy };
inline RivalStyle homeRival(int park) {
    return static_cast<RivalStyle>(park % 3);
}
inline const char *rivalName(RivalStyle style) {
    switch (style) {
    case RivalStyle::Slugger: return "THE SLUGGER";
    case RivalStyle::Umpire: return "THE UMPIRE";
    case RivalStyle::BallBoy: return "THE BALL BOY";
    }
    return "THE RIVAL";
}
inline int rivalPortrait(RivalStyle style) {
    return 1 + int(style);
}
struct RollResult {
    int knocked;
    bool strike, spare, split, gutter;
};
inline bool sevenTen(const Simulation &lane) {
    const auto pins = lane.standingPins();
    return lane.standingCount() == 2 && pins[6] && pins[9];
}
inline int scoreTotal(const Game &game) {
    int total = 0;
    for (auto score : game.scores())
        if (score)
            total = *score;
    return total;
}
inline const char *rivalReaction(RivalStyle style, RollResult roll, bool human) {
    switch (style) {
    case RivalStyle::Slugger:
        if (roll.strike) return human ? "YOU CAN HIT. SO CAN I." : "THAT ONE'S IN THE BLEACHERS!";
        if (roll.gutter) return human ? "WRONG SIDE OF THE FOUL LINE!" : "SWUNG TOO HARD.";
        if (roll.spare) return human ? "NICE CLEANUP." : "I'LL TAKE IT.";
        break;
    case RivalStyle::Umpire:
        if (roll.strike) return human ? "I CALL THAT A STRIKE." : "STRIKE. NO APPEAL.";
        if (roll.gutter) return human ? "FOUL BALL!" : "I MISSED THAT CALL.";
        if (roll.spare) return human ? "CLEAN CONVERSION." : "ONE PIN AT A TIME.";
        break;
    case RivalStyle::BallBoy:
        if (roll.strike) return human ? "WOW. CAN YOU SHOW ME THAT?" : "DID YOU SEE THAT?";
        if (roll.gutter) return human ? "HEH. I'LL GET IT!" : "I CAN STILL SAVE THE FRAME.";
        if (roll.spare) return human ? "THAT WAS CLOSE!" : "GOT 'EM!";
        break;
    }
    return "";
}
struct RivalTuning {
    float spread, power, steeringGain;
    int powerUpInterval;
    bool aimsAtStandingPin;
};
inline RivalTuning rivalTuning(RivalDifficulty difficulty) {
    switch (difficulty) {
    case RivalDifficulty::Rookie:
        return {3.4f, .42f, .12f, 0, false};
    case RivalDifficulty::Pro:
        return {1.15f, .72f, .50f, 3, true};
    case RivalDifficulty::AllStar:
        return {.22f, .98f, .85f, 2, true};
    }
    return {1.15f, .72f, .50f, 3, true};
}
struct Match {
    std::array<Game, 2> games{};
    std::array<Simulation, 2> lanes{};
    int player = 0, stage = 0;
    bool mascot = false;
    RivalDifficulty difficulty = RivalDifficulty::Pro;
    RivalStyle rival = RivalStyle::Slugger;
    std::array<int, 2> strikeStreak{};
    bool startedOnSplit = false;
    float rivalTarget = geometry::halfWidth;
    std::array<std::array<int, powerCount>, 2> stock{};
    std::array<Power, 2> queuedTrick{};
    Power activeTrick = Power::None;
    float wildPitchDirection = 0;
    Power equipped = Power::None;
    bool boostUsed = false;
    float steering = 0;
    std::mt19937 rng{981};
    bool complete() const {
        return games[0].complete && games[1].complete;
    }
    bool storePower(Power power) {
        int slot = int(power) - 1;
        if (slot < 0 || slot >= powerCount || stock[player][slot] == stockLimit)
            return false;
        ++stock[player][slot];
        return true;
    }
    bool canEquip(Power power) const {
        int slot = int(power) - 1;
        return slot >= 0 && slot < powerCount && !lanes[player].active && stock[player][slot] > 0 &&
               (!isTrick(power) || (!games[1 - player].complete && queuedTrick[1 - player] == Power::None));
    }
    bool equip(Power power) {
        if (!canEquip(power))
            return false;
        equipped = equipped == power ? Power::None : power;
        return true;
    }
    void launch(Shot s) {
        auto &sim = lanes[player];
        if (!sim.launch(s))
            return;
        boostUsed = false;
        startedOnSplit = sevenTen(sim);
        activeTrick = queuedTrick[player];
        queuedTrick[player] = Power::None;
        wildPitchDirection = sim.ball.x >= geometry::halfWidth ? -1.f : 1.f;
        if (mascot)
            sim.curve = 0;
        if (equipped != Power::None) {
            --stock[player][int(equipped) - 1];
            if (equipped == Power::Heavy)
                sim.ball.r = geometry::heavyBallRadius;
            if (equipped == Power::Fastball) {
                sim.ball.vx *= 1.18f;
                sim.ball.vz *= 1.18f;
            }
            if (isTrick(equipped))
                queuedTrick[1 - player] = equipped;
        }
        if (activeTrick == Power::Changeup)
            sim.ball.r *= .70f;
    }
    bool step(float dt) {
        auto &s = lanes[player];
        if (s.active) {
            if (activeTrick == Power::WildPitch && !s.gutter && s.elapsed < 1.15f)
                s.ball.vx += wildPitchDirection * .85f * dt;
            if (mascot && !s.gutter && s.ball.z < geometry::headPin - 1)
                s.ball.vx =
                    std::clamp((s.ball.vx + steering * 12 * dt) * std::exp(-1.2f * dt), -7.f, 7.f);
            if (equipped == Power::Magnet && !s.gutter &&
                s.ball.z > geometry::foulLine + (geometry::headPin - geometry::foulLine) * .5f &&
                !s.pins.empty()) {
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
                if (s.ball.x > geometry::width - edge) {
                    s.ball.x = geometry::width - edge;
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
    RollResult outcome() const {
        const auto &sim = lanes[player];
        const int pins = sim.knocked();
        const bool all = !sim.pins.empty() && pins == int(sim.pins.size());
        return {pins, all && pins == 10, all && pins < 10, all && startedOnSplit, sim.gutter};
    }
    void settle() {
        auto &g = games[player];
        auto &s = lanes[player];
        int old = g.frame;
        const auto roll = outcome();
        strikeStreak[player] = roll.strike ? strikeStreak[player] + 1 : 0;
        g.roll(roll.knocked);
        if (g.freshRack())
            s.resetRack();
        else
            s.clearFallen();
        s.hasBall = false;
        equipped = activeTrick = Power::None;
        steering = 0;
        if (g.complete || g.frame != old) {
            int other = 1 - player;
            if (!games[other].complete)
                player = other;
        }
    }
    Shot aiShot() {
        const auto tuning = rivalTuning(difficulty);
        const auto &pins = lanes[1].pins;
        float target = geometry::halfWidth;
        if (!pins.empty()) {
            target = 0;
            for (const auto &pin : pins)
                target += pin.x;
            target /= pins.size();
            if (tuning.aimsAtStandingPin || rival == RivalStyle::Umpire) {
                // Favor a real pin near the middle of the leave over the gap in a split.
                auto nearest =
                    std::min_element(pins.begin(), pins.end(), [target](const Body &a, const Body &b) {
                        return std::abs(a.x - target) < std::abs(b.x - target);
                    });
                target = nearest->x;
            }
            if (pins.size() == 10)
                target = geometry::halfWidth + .48f;
        }
        std::uniform_real_distribution<float> error(-1, 1);
        const float spread = tuning.spread * (1.f - .025f * stage);
        rivalTarget = std::clamp(target + error(rng) * spread, geometry::launchMin, geometry::launchMax);
        const float distance = geometry::headPin - geometry::foulLine;
        const bool fullRack = pins.size() == 10;
        const bool slugger = rival == RivalStyle::Slugger;
        const float powerStyle = slugger && fullRack ? .10f : rival == RivalStyle::Umpire ? -.08f : 0.f;
        const float noise = error(rng);
        const float power = std::clamp(tuning.power + powerStyle + error(rng) * spread * .045f, .15f, 1.f);
        if (mascot)
            return {rivalTarget, noise * spread / distance, power, 0};

        // A slugger starts wide and hooks toward the pocket. The umpire throws straight.
        const float curve = slugger && fullRack ? .62f : 0.f;
        const float position = std::clamp(rivalTarget - (curve > 0 ? 1.6f : 0.f),
                                          geometry::launchMin, geometry::launchMax);
        const float travel = (pins.empty() ? distance : pins.front().z - geometry::foulLine) /
                             geometry::launchSpeed(power);
        const float lateralDistance = rivalTarget - position - .5f * curve * .85f * travel * travel;
        const float variance = rival == RivalStyle::Umpire ? .35f : 1.f;
        const float aim = std::asin(std::clamp((lateralDistance + noise * spread * variance * .16f) /
                                                   (geometry::launchSpeed(power) * travel),
                                               -.15f, .15f)) / .16f;
        return {position, aim, power, curve};
    }
    void equipRivalPower() {
        const int interval = rivalTuning(difficulty).powerUpInterval;
        const int frame = games[1].frame;
        if (!interval || frame % interval != 1)
            return;
        bool fresh = games[1].freshRack();
        const Power favorite = rival == RivalStyle::Umpire ? (fresh ? Power::Changeup : Power::Magnet)
                               : rival == RivalStyle::Slugger ? (fresh ? Power::Heavy : Power::Fastball)
                                                              : (fresh ? Power::Bumpers : Power::WildPitch);
        if (equip(favorite))
            return;
        for (Power option : {Power::Fastball, Power::Heavy, Power::Bumpers, Power::Magnet,
                             Power::Changeup, Power::WildPitch})
            if (equip(option))
                return;
    }
    float rivalSteering() const {
        const auto &ball = lanes[1].ball;
        float gain = rivalTuning(difficulty).steeringGain;
        return std::clamp((rivalTarget - ball.x) * gain - ball.vx * .35f, -1.f, 1.f);
    }
};
} // namespace bowling
