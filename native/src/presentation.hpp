#pragma once
#include "campaign.hpp"
#include <optional>

namespace bowling {
struct LaneToken {
    float x, z;
};
inline bool crossedPickup(const Body &from, const Simulation &lane, LaneToken token, float reach) {
    if (lane.gutter || token.z <= from.z || token.z > lane.ball.z)
        return false;
    const float fraction = (token.z - from.z) / (lane.ball.z - from.z);
    const float x = from.x + fraction * (lane.ball.x - from.x);
    return std::abs(token.x - x) <= reach + lane.ball.r;
}

struct PowerPickup {
    LaneToken position;
    Power power;
    bool claimed = false;
};
// Each row describes successive frames after a drop or the start of a match.
struct PickupTuning {
    std::array<int, 10> chances;
    int matchLimit;
    std::array<int, powerCount> weights;
};

inline constexpr std::array<PickupTuning, rivalDifficultyCount> pickupTuning{{
    {{50, 95, 100}, 10, {18, 20, 18, 16, 14, 14}},
    {{0, 50, 95, 100}, 10, {12, 10, 12, 24, 21, 21}},
    {{3, 5, 10, 18, 35, 65, 90, 100}, 2, {6, 4, 6, 32, 26, 26}},
    {{1, 2, 4, 7, 12, 20, 35, 60, 85, 100}, 1, {3, 2, 3, 36, 28, 28}},
}};

struct PowerPickups {
    static constexpr float radius = .5f;
    std::optional<PowerPickup> item;

    void startMatch(RivalDifficulty level, std::mt19937::result_type seed) {
        difficulty = level;
        rng.seed(seed);
        item.reset();
        currentFrame = -1;
        missedFrames = spawned = 0;
    }
    void enterFrame(int frame) {
        if (frame <= currentFrame)
            return;
        currentFrame = frame;
        item.reset();
        const auto &tuning = pickupTuning[int(difficulty)];
        if (spawned == tuning.matchLimit)
            return;

        // A drop resets the streak even if the bowler leaves the medal unclaimed.
        const int chance = tuning.chances[missedFrames];
        std::uniform_int_distribution<int> percent(1, 100);
        if (percent(rng) > chance) {
            ++missedFrames;
            return;
        }
        missedFrames = 0;
        ++spawned;
        std::discrete_distribution<int> kind(tuning.weights.begin(), tuning.weights.end());
        std::uniform_real_distribution<float> side(1.65f, geometry::width - 1.65f);
        std::uniform_real_distribution<float> depth(30.f, 144.f);
        item = PowerPickup{{side(rng), depth(rng)}, Power(1 + kind(rng))};
    }
    Power sweep(const Body &from, Match &match) {
        const auto &lane = match.lanes[match.player];
        if (!item || item->claimed || !crossedPickup(from, lane, item->position, radius) ||
            !match.storePower(item->power))
            return Power::None;
        item->claimed = true;
        return item->power;
    }

  private:
    std::mt19937 rng;
    RivalDifficulty difficulty = RivalDifficulty::Pro;
    int currentFrame = -1, missedFrames = 0, spawned = 0;
};

struct MascotTokens {
    static constexpr std::array<LaneToken, 3> layout{{{4.5f, 43.f}, {6.f, 87.f}, {4.f, 126.f}}};
    static constexpr float reach = .7f;
    std::array<bool, layout.size()> collected{};
    int count = 0;

    void sweep(const Body &from, const Simulation &lane) {
        for (size_t i = 0; i < layout.size(); ++i)
            if (!collected[i] && crossedPickup(from, lane, layout[i], reach)) {
                collected[i] = true;
                ++count;
            }
    }
};

inline void setSevenTenRack(Simulation &lane) {
    lane.resetRack();
    lane.pins.erase(std::remove_if(lane.pins.begin(), lane.pins.end(),
                                   [](const Body &pin) { return pin.id != 6 && pin.id != 9; }),
                    lane.pins.end());
}

enum class StadiumCue { None, Turkey, TropicanaUnlocked };
inline StadiumCue strikeCue(int park, bool mascot, int previousStreak, RollResult roll) {
    return park == 9 && !mascot && roll.strike && previousStreak == 2
               ? StadiumCue::Turkey
               : StadiumCue::None;
}
inline bool closeFinish(int yours, int theirs) {
    return std::abs(yours - theirs) <= 10;
}

// Playback only reads snapshots. Live rack, timers, powers and score stay untouched.
struct PinReplay {
    static constexpr float sampleStep = 1.f / 30;
    static constexpr float playbackRate = .65f;
    static constexpr float maxSeconds = 4.f;
    std::vector<Simulation> frames;
    int ticksToSample = 0;
    size_t finalImpact = 0;
    int knocked = 0;
    float seconds = 0;

    void reset() {
        frames.clear();
        ticksToSample = 0;
        finalImpact = 0;
        knocked = 0;
        seconds = 0;
    }
    void record(const Simulation &lane, bool finished) {
        if (lane.ball.z < geometry::headPin - 15.f)
            return;
        if (++ticksToSample == 4 || frames.empty() || finished) {
            const int newCount = lane.knocked();
            if (newCount > knocked) {
                knocked = newCount;
                finalImpact = frames.size();
            }
            frames.push_back(lane);
            ticksToSample = 0;
        }
    }
    bool available(RollResult roll) const {
        return (roll.strike || roll.split) && frames.size() > 1;
    }
    size_t lastFrame() const {
        // Allow the final pin to tip over before cutting away.
        return std::min(frames.size() - 1, finalImpact + size_t(.45f / sampleStep));
    }
    size_t firstFrame() const {
        const size_t span = size_t(maxSeconds * playbackRate / sampleStep);
        return lastFrame() > span ? lastFrame() - span : 0;
    }
    bool finished() const {
        return seconds >= (lastFrame() - firstFrame()) * sampleStep / playbackRate + .18f;
    }
    const Simulation &view() const {
        const size_t index = firstFrame() + size_t(seconds * playbackRate / sampleStep);
        return frames[std::min(index, lastFrame())];
    }
};

struct MascotPose {
    float pull = 0;     // 0 at rest, 1 fully drawn.
    float sideways = 0; // Lateral displacement as a fraction of maximum.
    float recoil = 0;   // Signed damped release spring.
    float bank = 0;     // Lateral roll intent, -1 to 1.
    float wipeout = 0;  // Progress from upright to wiped out.
};
struct MascotMotion {
    MascotPose pose;
    float releaseSeconds = 0, releaseFrom = 0;

    void aim(float pull, float sideways) {
        pose = {};
        pose.pull = pull;
        pose.sideways = sideways;
    }
    void release() {
        releaseFrom = pose.pull;
        releaseSeconds = 0;
        pose = {};
    }
    void update(float dt, const Simulation &lane) {
        releaseSeconds += dt;
        pose.recoil = releaseFrom * std::exp(-9.f * releaseSeconds) *
                      std::cos(24.f * releaseSeconds);
        const float target = std::clamp(lane.ball.vx / 6.f, -1.f, 1.f);
        pose.bank += (target - pose.bank) * std::min(1.f, dt * 9.f);
        if (lane.gutter)
            pose.wipeout = std::min(1.f, pose.wipeout + dt * 2.2f);
    }
};
} // namespace bowling
