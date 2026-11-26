#pragma once
#include "game.hpp"
#include "teams.hpp"
#include <cstdint>
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
    int avatar = 0, bestStraightRide = 0, bestPowerRide = 0;
    int xp = 0, wins = 0, matches = 0, best = 0, equippedBall = 0;
    unsigned cleared = 0, completedChallenges = 0;
    std::uint64_t unseenBalls = 0;
    bool completed(Challenge challenge) const {
        return (completedChallenges & challengeBit(challenge)) != 0;
    }
    bool finish(Challenge challenge) {
        if (completed(challenge))
            return false;
        for (const auto &reward : challenges)
            if (reward.challenge == challenge && !ballUnlocked(reward.ball))
                unseenBalls |= std::uint64_t{1} << reward.ball;
        completedChallenges |= challengeBit(challenge);
        return true;
    }
    bool ballUnlocked(int ball) const {
        const auto *reward = ballChallenge(ball);
        return ball >= 0 && ball < 64 &&
               (xp >= ball * 80 || (reward && completed(reward->challenge)));
    }
    std::uint64_t unlockedMask() const {
        std::uint64_t mask = 0;
        for (int id = 0; id < 64; ++id)
            if (ballUnlocked(id)) mask |= std::uint64_t{1} << id;
        return mask;
    }
    int nextShowcase() const {
        for (int id = 0; id < 64; ++id)
            if (unseenBalls & (std::uint64_t{1} << id)) return id;
        return -1;
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
            magic != "BLB" || (version < 1 || version > 6) || p.xp < 0 || p.xp > 100000000 || p.wins < 0 ||
            p.matches < p.wins || p.best < 0 || p.best > 300 || p.cleared > 1023)
            return false;
        if (version >= 2 && (!(f >> p.completedChallenges) || p.completedChallenges > 7))
            return false;
        if (version >= 3 && (!(f >> p.avatar) || p.avatar < 0 || p.avatar >= 6))
            return false;
        if (version >= 4 && (!(f >> p.bestStraightRide >> p.bestPowerRide) ||
            p.bestStraightRide < 0 || p.bestStraightRide > 300 ||
            p.bestPowerRide < 0 || p.bestPowerRide > 420))
            return false;
        if (version >= 5 && (!(f >> p.equippedBall) || !p.ballUnlocked(p.equippedBall)))
            return false;
        if (version >= 6 && (!(f >> p.unseenBalls) || (p.unseenBalls & ~p.unlockedMask())))
            return false;
        *this = p;
        return true;
    }
    bool save(const std::string &path) const {
        std::error_code ec;
        const auto directory = std::filesystem::path(path).parent_path();
        if (!directory.empty()) {
            std::filesystem::create_directories(directory, ec);
            if (ec)
                return false;
        }
        const auto temp = path + ".tmp";
        {
            std::ofstream f(temp);
            f << "BLB 6 " << xp << ' ' << wins << ' ' << matches << ' ' << best << ' ' << cleared
              << ' ' << completedChallenges << ' ' << avatar << ' ' << bestStraightRide << ' '
              << bestPowerRide << ' ' << equippedBall << ' ' << unseenBalls << '\n';
            f.close();
            if (!f)
                return false;
        }
        Profile previous;
        if (previous.load(path)) {
            const auto backup = path + ".bak";
            const auto stagedBackup = backup + ".tmp";
            std::filesystem::copy_file(path, stagedBackup, std::filesystem::copy_options::overwrite_existing, ec);
            if (ec)
                return false;
            std::filesystem::rename(stagedBackup, backup, ec);
            if (ec)
                return false;
        }
        std::filesystem::rename(temp, path, ec);
        return !ec;
    }
    int award(int score, int opponent, int arena, int bonus = 0, int opponentBonus = 0) {
        ++matches;
        best = std::max(best, score);
        bool win = score + bonus > opponent + opponentBonus;
        if (win) {
            ++wins;
            cleared |= 1u << arena;
        }
        int gained = 120 + score + (win ? 150 : 0);
        const auto before = unlockedMask();
        xp = std::min(100000000, xp + gained);
        unseenBalls |= unlockedMask() & ~before;
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
enum class RivalDifficulty { Rookie, Pro, AllStar, HallOfFame };
inline constexpr int rivalDifficultyCount = int(RivalDifficulty::HallOfFame) + 1;
inline const char *difficultyName(RivalDifficulty difficulty) {
    switch (difficulty) {
    case RivalDifficulty::Rookie:
        return "ROOKIE";
    case RivalDifficulty::Pro:
        return "PRO";
    case RivalDifficulty::AllStar:
        return "ALL-STAR";
    case RivalDifficulty::HallOfFame:
        return "HALL OF FAME";
    }
    return "PRO";
}
enum class RivalStyle { Slugger, Umpire, BallBoy };
struct Bowler { const char *name; RivalStyle style; };
inline constexpr std::array<Bowler, 25> roster{{
    {"Evan Longoria", RivalStyle::Slugger},
    {"Carl Crawford", RivalStyle::BallBoy},
    {"David Price", RivalStyle::Umpire},
    {"Raymond", RivalStyle::BallBoy},
    {"DJ Kitty", RivalStyle::BallBoy},
    {"Rookie Catcher", RivalStyle::Umpire},
    {"Pete Crow-Armstrong", RivalStyle::BallBoy},
    {"Roman Anthony", RivalStyle::Slugger},
    {"Logan Webb", RivalStyle::Umpire},
    {"Paul Skenes", RivalStyle::Umpire},
    {"Ezequiel Tovar", RivalStyle::BallBoy},
    {"Bobby Witt Jr.", RivalStyle::BallBoy},
    {"Corbin Carroll", RivalStyle::BallBoy},
    {"Julio Rodriguez", RivalStyle::Slugger},
    {"Aaron Judge", RivalStyle::Slugger},
    {"Ernie Banks", RivalStyle::Slugger},
    {"David Ortiz", RivalStyle::Slugger},
    {"Willie Mays", RivalStyle::Slugger},
    {"Roberto Clemente", RivalStyle::BallBoy},
    {"Todd Helton", RivalStyle::Slugger},
    {"George Brett", RivalStyle::Umpire},
    {"Randy Johnson", RivalStyle::Umpire},
    {"Ken Griffey Jr.", RivalStyle::Slugger},
    {"Derek Jeter", RivalStyle::Umpire},
    {"Junior Caminero", RivalStyle::Slugger},
}};
inline int drawHomeRival(int park, std::mt19937 &rng) {
    if (park == 9) {
        constexpr int tropicana[] = {24, 0, 3}; // Caminero, Longoria and Raymond.
        return tropicana[std::uniform_int_distribution<int>(0, 2)(rng)];
    }
    return 6 + park + 9 * std::uniform_int_distribution<int>(0, 1)(rng);
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
    case RivalDifficulty::HallOfFame:
        return {.065f, 1.f, 1.05f, 2, true};
    }
    return {1.15f, .72f, .50f, 3, true};
}
struct Match {
    std::array<Game, 2> games{};
    std::array<Simulation, 2> lanes;
    int player = 0, stage = 0;
    bool mascot;
    RivalDifficulty difficulty = RivalDifficulty::Pro;
    RivalStyle rival = RivalStyle::Slugger;
    std::array<int, 2> strikeStreak{}, rideBonus{};
    bool startedOnSplit = false;
    float rivalTarget = geometry::halfWidth;
    std::array<std::array<int, powerCount>, 2> stock{};
    std::array<Power, 2> queuedTrick{};
    Power activeTrick = Power::None;
    float wildPitchDirection = 0;
    Power equipped = Power::None;
    float steering = 0;
    std::mt19937 rng{981};
    explicit Match(bool ride = false)
        : lanes{Simulation(ride ? geometry::mascotLane : geometry::stadiumLane),
                Simulation(ride ? geometry::mascotLane : geometry::stadiumLane)}, mascot(ride) {}
    bool complete() const {
        return games[0].complete && games[1].complete;
    }
    int total(int bowler) const { return scoreTotal(games[bowler]) + rideBonus[bowler]; }
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
        if (mascot)
            s.aim = s.curve = 0;
        auto &sim = lanes[player];
        if (!sim.launch(s))
            return;
        startedOnSplit = sevenTen(sim);
        activeTrick = queuedTrick[player];
        queuedTrick[player] = Power::None;
        wildPitchDirection = sim.ball.x >= geometry::halfWidth ? -1.f : 1.f;
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
            if (mascot && !s.gutter && s.ball.z < s.layout.headPin() - 1)
                s.ball.vx =
                    std::clamp((s.ball.vx + steering * 12 * dt) * std::exp(-1.2f * dt), -7.f, 7.f);
            if (equipped == Power::Magnet && !s.gutter && s.ball.z > s.layout.at(.5f)) {
                const Body *target = nullptr;
                for (const auto &pin : s.pins)
                    if (!pin.down && (!target || std::abs(pin.x - s.ball.x) < std::abs(target->x - s.ball.x)))
                        target = &pin;
                if (target)
                    s.ball.vx += (target->x - s.ball.x) * 2 * dt;
            }
        }
        return s.step(dt, equipped == Power::Bumpers);
    }
    RollResult outcome() const {
        const auto &sim = lanes[player];
        const int pins = sim.knocked();
        const bool all = !sim.pins.empty() && pins == int(sim.pins.size());
        const bool strike = all && pins == 10 && games[player].freshRack();
        return {pins, strike, all && !strike, all && startedOnSplit, sim.gutter};
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
            if (pins.size() == 10) {
                // The faster Hall of Fame shots need their own pocket line.
                const float pocket = mascot ? .65f
                                     : difficulty == RivalDifficulty::HallOfFame ? .54f : .48f;
                target = geometry::halfWidth + pocket;
            }
        }
        std::uniform_real_distribution<float> error(-1, 1);
        const float spread = tuning.spread * (1.f - .025f * stage);
        rivalTarget = std::clamp(target + error(rng) * spread, geometry::launchMin, geometry::launchMax);
        const float distance = lanes[1].layout.run;
        const bool fullRack = pins.size() == 10;
        const bool slugger = rival == RivalStyle::Slugger;
        const float powerStyle = slugger && fullRack ? .10f : rival == RivalStyle::Umpire ? -.08f : 0.f;
        const float noise = error(rng);
        const float power = std::clamp(tuning.power + powerStyle + error(rng) * spread * .045f, .15f, 1.f);
        if (mascot)
            return {rivalTarget, 0, power, 0};

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
    float rivalSteering(float target) const {
        const auto &ball = lanes[1].ball;
        float gain = rivalTuning(difficulty).steeringGain;
        return std::clamp((target - ball.x) * gain - ball.vx * .35f, -1.f, 1.f);
    }
};
} // namespace bowling
