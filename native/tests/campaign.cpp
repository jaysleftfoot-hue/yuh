#include "presentation.hpp"
#include "alley_controls.hpp"
#include <iostream>
#include <set>
using namespace bowling;
void require(bool b, const char *s) {
    if (!b)
        throw std::runtime_error(s);
}
int main() {
    for (auto lane : {geometry::stadiumLane, geometry::mascotLane}) {
        Simulation rack(lane);
        Body fallen = rack.pins.front();
        fallen.down = true;
        PinsetterCycle cycle;
        require(cycle.pinZ(fallen, lane) == fallen.z && cycle.lift() == 0, "rack stays put at release");
        cycle.progress = .36f;
        require(cycle.bladeZ(lane) < lane.headPin() && cycle.lift() > 1,
                "spare pins lift before the low sweep moves back");
        cycle.progress = .8f;
        require(cycle.pinZ(fallen, lane) > lane.surfaceEnd() &&
                cycle.pinZ(rack.pins.back(), lane) == rack.pins.back().z,
                "only fallen pins travel to the pit");
        cycle.progress = 1;
        require(cycle.lift() == 0 && cycle.lowered() == 0, "pinsetter parks for the next delivery");
    }

    Shot input{};
    input.position = 3;
    input.curve = .2f;
    StadiumSetup setup;
    setup.adjust(input, 7, -.9f);
    require(input.position == 7 && input.curve == .2f, "spot changes only position");
    require(!setup.confirm() && setup.phase == ShotPhase::Curve, "first confirm cannot launch");
    setup.adjust(input, 1, -.6f);
    require(input.position == 7 && input.curve == -.6f, "curve locks spot");
    require(!setup.confirm() && setup.phase == ShotPhase::Power, "second confirm cannot launch");
    setup.adjust(input, 2, .9f);
    setup.tick(input, .6f);
    require(input.position == 7 && input.curve == -.6f && input.power > .8f,
            "power locks spot and curve");
    require(setup.confirm(), "only third confirm launches");
    setup.back();
    require(setup.phase == ShotPhase::Curve, "back edits curve before launch");
    setup.reset();
    require(setup.phase == ShotPhase::Spot && setup.seconds == 0, "new shot resets all phases");
    require(alleyPosition(-100) == geometry::launchMin && alleyPosition(2000) == geometry::launchMax,
            "placement clamps to lane");
    input.aim = .7f;
    auto pull = mascotPull(input, 80);
    require(std::abs(pull.power - .65f) < .001f && pull.aim == 0,
            "pull length sets speed and the sling launches straight");
    require(pull.position == input.position && pull.curve == 0, "the sling locks the selected spot");
    require(mascotPull(input, 1000).power == 1 && mascotPull(input, -90).power == .15f,
            "sling pull is clamped in both directions");

    require(mascotMouseDraw(542, 622, 798) == 80, "normal sling travel stays unchanged");
    require(mascotPull(input, mascotMouseDraw(750, 798, 798)).power == 1,
            "low grip reaches full power before the window edge");

    Match spareAfterMiss(true);
    spareAfterMiss.settle();
    for (auto &pin : spareAfterMiss.lanes[0].pins)
        pin.down = true;
    const auto recovery = spareAfterMiss.outcome();
    require(recovery.spare && !recovery.strike && recovery.knocked == 10,
            "a clean pickup after a miss is a spare");
    spareAfterMiss.settle();
    require(spareAfterMiss.games[0].marks(0) == "- /" && spareAfterMiss.strikeStreak[0] == 0,
            "reaction, streak and scorecard agree on the spare");

    Match tenthSpare(true);
    tenthSpare.games[0].frame = 9;
    tenthSpare.games[0].roll(10);
    tenthSpare.games[0].roll(0);
    for (auto &pin : tenthSpare.lanes[0].pins)
        pin.down = true;
    const auto bonusSpare = tenthSpare.outcome();
    require(bonusSpare.spare && !bonusSpare.strike, "tenth frame X - / is a spare on the last ball");

    std::mt19937 rideRng(61);
    RidePickups route;
    require(!route.route && route.bonus(10) == 0, "rider starts without a visible route or powers");
    route.reveal(rideRng);
    const auto first = route.route->front();
    route.reveal(rideRng);
    require(route.route->front().position.x == first.position.x && route.route->front().power == first.power,
            "regrabbing uses the same route for the shot");
    Simulation ride(geometry::mascotLane);
    ride.launch(pull);
    for (const auto &item : *route.route) {
        auto before = ride.ball;
        before.x = ride.ball.x = item.position.x;
        before.z = item.position.z - 1;
        ride.ball.z = item.position.z + 1;
        ride.gutter = true;
        require(route.sweep(before, ride) == RidePower::None, "gutter cannot claim ride power");
        ride.gutter = false;
        require(route.sweep(before, ride) == item.power && route.sweep(before, ride) == RidePower::None,
                "crossing the coin claims it once");
    }
    require(route.bonus(6) == 6 && route.bonus(0) == 0, "2x adds only this shot's pin count");
    ride.ball.x = geometry::halfWidth;
    ride.ball.z = geometry::mascotLane.headPin();
    require(!route.impact(ride, 0), "wave waits for contact");
    ride.elapsed = 3.4f;
    ++ride.impacts;
    require(route.impact(ride, 0) && ride.knocked() >= 2 && !route.impact(ride, 0),
            "first impact triggers the wave once against nearby pins");
    require(route.wave->progress(3.2f) < 0 && route.wave->progress(3.5f) > 0 &&
                route.wave->progress(4.2f) > 1 && route.wave->center.z == ride.ball.z,
            "wave follows the sampled shot clock and original contact point during replay");
    route = {};
    require(route.bonus(6) == 0 && !route.impact(ride, 0), "ride powers expire before the next shot");
    Profile p;
    require(p.balls() == 1 && p.stadiums() == 1, "initial unlocks");
    Profile showcase;
    showcase.award(0, 1, 0);
    require(showcase.nextShowcase() == 1, "first earned ball waits in locker");
    showcase.unseenBalls = 0;
    showcase.finish(Challenge::FenwaySplit);
    require(showcase.nextShowcase() == 33, "challenge ball enters the same showcase");
    showcase.unseenBalls = 0;
    showcase.xp = 33*80;
    showcase.finish(Challenge::WrigleyWin);
    showcase.unseenBalls = 0;
    showcase.award(0, 1, 0);
    require(showcase.nextShowcase() == -1, "XP does not repeat an acknowledged challenge ball");
    showcase.xp = 5039;
    showcase.award(0, 1, 0);
    require(showcase.nextShowcase() == 63, "last design survives the 64-bit boundary");
    std::set<std::string> names;
    for (int i = 0; i < 64; ++i)
        names.insert(ballName(i));
    require(names.size() == 64, "64 distinct designs");
    require(std::string(arenas[9].name) == "Tropicana Field", "Tropicana finale");
    require(p.finish(Challenge::WrigleyWin) && !p.finish(Challenge::WrigleyWin), "challenge completion is one-time");
    require(p.ballUnlocked(34) && !p.ballUnlocked(33) && p.balls() == 2,
            "challenge unlocks its specific ball without opening intervening slots");
    p.award(100, 150, 0);
    require(p.stadiums() == 1 && p.balls() > 1, "loss gives XP, no stadium");
    for (int arena = 0; arena < 9; ++arena) {
        p.award(180, 100, arena);
        require(p.stadiums() == arena + 2, "ordered stadium unlock");
    }
    p.xp = 5040;
    require(p.balls() == 64, "all balls at 5040 XP");
    Match scoredRide(true);
    scoredRide.games[0].roll(4);
    scoredRide.games[0].roll(4);
    scoredRide.games[1].roll(9);
    scoredRide.games[1].roll(0);
    scoredRide.rideBonus = {4, 0};
    require(scoreTotal(scoredRide.games[0]) == 8 && scoredRide.total(0) == 12 && scoredRide.total(1) == 9,
            "ride bonus enters the match total but never the scorecard");
    Profile rideCareer;
    require(rideCareer.award(8, 9, 0, 4, 0) == 278 && rideCareer.wins == 1 &&
            rideCareer.best == 8 && rideCareer.stadiums() == 2,
            "a bonus win unlocks the park while keeping base pin score and XP");
    auto path = (std::filesystem::temp_directory_path() / "blb-campaign-test.txt").string();
    require(p.save(path), "save");
    Profile restored;
    std::mt19937 rosterRng(54);
    for (int park = 0; park < 10; ++park) {
        std::array<bool, roster.size()> seen{};
        for (int draw = 0; draw < 160; ++draw) seen.at(drawHomeRival(park, rosterRng)) = true;
        require(park == 9 ? seen[0] && seen[3] && seen[24] : seen[6+park] && seen[15+park],
                "home roster includes both eras and Raymond at Tropicana");
        require(std::count(seen.begin(), seen.end(), true) == (park == 9 ? 3 : 2), "home roster only");
    }
    require(restored.load(path) && restored.xp == p.xp && restored.cleared == p.cleared &&
                restored.unseenBalls == p.unseenBalls && restored.completed(Challenge::WrigleyWin), "restore challenge profile");
    p.xp += 80;
    p.avatar = 4;
    p.bestStraightRide = 165;
    p.bestPowerRide = 319;
    p.equippedBall = 63;
    require(p.save(path) && restored.load(path) && restored.avatar == 4 && restored.xp == p.xp &&
                restored.bestStraightRide == 165 && restored.bestPowerRide == 319 && restored.equippedBall == 63, "save replacement");
    {
        std::ofstream f(path);
        f << "BLB 1 80 1 2 100 1";
    }
    require(restored.load(path) && restored.xp == 80 && restored.stadiums() == 2 &&
                restored.unseenBalls == 0 && restored.completedChallenges == 0 && restored.equippedBall == 0, "load existing version 1 save");
    {
        std::ofstream f(path);
        f << "BLB 5 160 1 2 100 1 0 0 0 0 2";
    }
    require(restored.load(path) && restored.equippedBall == 2 && restored.nextShowcase() < 0,
            "upgrading keeps equipped design and skips retroactive unlock showcases");
    {
        std::ofstream f(path);
        f << "BLB 6 0 0 0 0 0 0 0 0 0 0 8";
    }
    require(!restored.load(path), "locked balls cannot be queued in a save");
    {
        std::ofstream f(path);
        f << "BLB 1 -1 0 0 300 0";
    }
    require(!restored.load(path), "reject corrupt save");
    require(restored.load(path + ".bak") && restored.balls() == 64, "recover previous career");
    require(restored.save(path), "write recovered career");
    require(restored.load(path) && restored.load(path + ".bak") && restored.balls() == 64,
            "recovery must preserve the good backup when the main file is corrupt");
    std::filesystem::remove(path);
    std::filesystem::remove(path + ".bak");
    Match m;
    require(!m.equip(Power::Heavy), "power inventory starts empty");
    require(m.storePower(Power::Heavy) && m.equip(Power::Heavy), "equip collected heavy");
    m.launch({5, 0, .7f, 0});
    require(m.stock[0][0] == 0 && m.lanes[0].ball.r > .8f, "heavy consumes one charge");
    require(!m.equip(Power::Magnet), "no equipping mid-flight");
    Match a(true), b(true);
    a.launch({5, 0, .7f, 0});
    b.launch({5, 0, .7f, 0});
    a.steering = 1;
    for (int i = 0; i < 30; ++i) {
        a.step(1.f / 120);
        b.step(1.f / 120);
    }
    require(a.lanes[0].ball.x > b.lanes[0].ball.x, "mascot steering changes path");
    require(a.lanes[0].ball.vz == b.lanes[0].ball.vz, "steering preserves sling speed");
    Match lateSteering(true);
    lateSteering.launch({5, 0, .7f, 0});
    lateSteering.lanes[0].ball.z = geometry::mascotLane.headPin() - 12;
    lateSteering.steering = 1;
    lateSteering.step(1.f / 120);
    require(lateSteering.lanes[0].ball.vx > 0, "mascot steering remains active near distant pin deck");
    Match bumper;
    bumper.storePower(Power::Bumpers);
    bumper.equip(Power::Bumpers);
    bumper.launch({.7f, -1, .7f, 0});
    for (int i = 0; i < 150; ++i) {
        bumper.step(1.f / 120);
        require(!bumper.lanes[0].gutter, "bumper never reports a gutter contact");
    }
    require(bumper.lanes[0].ball.x > 3 && bumper.lanes[0].ball.vx > 0, "bumper reflects a side hit");
    Match magnet;
    magnet.storePower(Power::Magnet);
    magnet.equip(Power::Magnet);
    magnet.launch({5, 0, .7f, 0});
    auto &magnetLane = magnet.lanes[0];
    magnetLane.ball.z = geometry::headPin - 4;
    for (auto &pin : magnetLane.pins)
        pin.down = pin.x <= magnetLane.ball.x;
    magnet.step(1.f / 120);
    require(magnetLane.ball.vx > 0, "magnet aims past fallen pins toward the standing rack");
    Match rack;
    auto &lane = rack.lanes[0];
    lane.pins[2].down = true;
    lane.pins[9].down = true;
    auto standing = lane.standingPins();
    require(lane.standingCount() == 8 && !standing[2] && !standing[9],
            "HUD shows fallen pins before the roll is scored");
    rack.settle();
    require(lane.standingPins() == standing && lane.pins.size() == 8, "spare keeps original pin IDs");
    rack.settle();
    require(rack.player == 1 && rack.lanes[1].standingCount() == 10, "rival gets its own rack");
    Match drill;
    auto &splitLane = drill.lanes[0];
    setSevenTenRack(splitLane);
    require(sevenTen(splitLane), "split drill leaves only the two corner pins");
    PinReplay replay;
    drill.launch({2.f, 0.f, 1.f, 0.f});
    while (splitLane.active) {
        const bool finished = drill.step(1.f / 120);
        MascotPose pose;
        pose.bank = std::sin(splitLane.elapsed);
        replay.record(splitLane, finished, pose);
    }
    require(drill.outcome().split && drill.outcome().spare, "inside-edge bank can convert the 7-10");
    require(replay.available(drill.outcome()) && replay.view().knocked() < 2, "split replay starts before impact");
    require(replay.frame().rider.bank == std::sin(replay.view().elapsed), "replay pose matches the displayed tick");
    replay.seconds = PinReplay::maxSeconds;
    require(replay.view().knocked() == 2 && splitLane.standingCount() == 0 &&
                drill.games[0].frames[0].empty(), "replay shows both falls without advancing live score");
    require(strikeCue(9, false, 2, {10, true, false, false, false}) == StadiumCue::Turkey &&
                strikeCue(9, true, 2, {10, true, false, false, false}) == StadiumCue::None &&
                strikeCue(0, false, 2, {10, true, false, false, false}) == StadiumCue::None,
            "turkey cue is specific to Tropicana stadium");

    Match battingPractice;
    battingPractice.lanes[0].ball = {5, 50};
    PowerPickups pickups;
    pickups.item = PowerPickup{{7.9f, 48}, Power::Fastball};
    Body before{5, 47};
    require(pickups.sweep(before, battingPractice) == Power::None, "wide miss collects nothing");
    pickups.item = PowerPickup{{5, 48}, Power::Fastball};
    require(pickups.sweep(before, battingPractice) == Power::Fastball &&
                battingPractice.stock[0][int(Power::Fastball) - 1] == 1 &&
                pickups.sweep(before, battingPractice) == Power::None,
            "swept pickup goes into active inventory once");
    require(battingPractice.equip(Power::Fastball), "equip pickup next shot");
    battingPractice.launch({5, 0, 1, 0});
    require(battingPractice.lanes[0].ball.vz > geometry::maxLaunchSpeed, "Fastball increases speed");

    Match trick;
    trick.storePower(Power::Changeup);
    require(trick.equip(Power::Changeup), "equip opponent trick");
    trick.launch({});
    require(trick.lanes[0].ball.r == geometry::ballRadius && trick.queuedTrick[1] == Power::Changeup,
            "trick queues against opponent without affecting owner's shot");
    trick.player = 1;
    trick.equipped = Power::None;
    trick.launch({});
    require(trick.lanes[1].ball.r < geometry::ballRadius && trick.queuedTrick[1] == Power::None,
            "Changeup shrinks only opponent's next shot");
    trick.lanes[1].active = false;
    trick.launch({});
    require(trick.lanes[1].ball.r == geometry::ballRadius, "trick expires after one shot");

    Match drift;
    drift.queuedTrick[0] = Power::WildPitch;
    drift.launch({5, 0, .72f, 0});
    for (int i = 0; i < 60; ++i)
        drift.step(1.f / 120);
    require(drift.lanes[0].ball.x < 5 && drift.queuedTrick[0] == Power::None,
            "Wild Pitch deflects one shot");

    require(pickupTuning[0].chances[0] == 50 && pickupTuning[0].chances[1] == 95 &&
                pickupTuning[0].chances[2] == 100, "rookie pity curve");
    std::mt19937 rng{109};
    std::set<Power> types;
    float previousRareRate = 1.f;
    int previousDropTotal = 10000;
    constexpr std::array<int, rivalDifficultyCount> longestGap{3, 4, 8, 10};
    for (int tier = 0; tier < rivalDifficultyCount; ++tier) {
        int total = 0, rare = 0;
        for (int game = 0; game < 1000; ++game) {
            pickups.startMatch(RivalDifficulty(tier), rng());
            int count = 0, lastDrop = -1;
            for (int f = 0; f < 10; ++f) {
                pickups.enterFrame(f);
                if (!pickups.item)
                    continue;
                require(f - lastDrop <= longestGap[tier], "pickup drought ends at guaranteed frame");
                lastDrop = f;
                ++count;
                const auto power = pickups.item->power;
                types.insert(power);
                rare += power == Power::Heavy || power == Power::Magnet || power == Power::Bumpers;
                pickups.item->claimed = true;
                pickups.enterFrame(f);
                require(pickups.item && pickups.item->claimed && pickups.item->power == power,
                        "spare and bonus attempts keep the same pickup roll");
            }
            require(count >= 1 && count <= pickupTuning[tier].matchLimit, "per-bowler pickup budget");
            total += count;
        }
        const float rareRate = float(rare) / total;
        require(total < previousDropTotal && rareRate < previousRareRate,
                "higher difficulty reduces medals and strong drops");
        previousDropTotal = total;
        previousRareRate = rareRate;
    }
    require(types.size() == powerCount, "all six powers can spawn");

    PowerPickups visitor, visitorControl, home;
    visitor.startMatch(RivalDifficulty::Rookie, 3141);
    visitorControl.startMatch(RivalDifficulty::Rookie, 3141);
    home.startMatch(RivalDifficulty::Rookie, 5926);
    bool differentLanes = false;
    for (int frame = 0; frame < 10; ++frame) {
        visitor.enterFrame(frame);
        home.enterFrame(frame);
        visitorControl.enterFrame(frame);
        require(bool(visitor.item) == bool(visitorControl.item), "rival never advances visitor RNG");
        if (visitor.item) {
            require(visitor.item->position.x == visitorControl.item->position.x &&
                        visitor.item->position.z == visitorControl.item->position.z &&
                        visitor.item->power == visitorControl.item->power,
                    "rival turns preserve visitor position and power draws");
            visitor.item->claimed = true;
        }
        if (bool(visitor.item) != bool(home.item) ||
            (visitor.item && home.item && visitor.item->position.x != home.item->position.x))
            differentLanes = true;
        if (visitor.item && home.item)
            require(!home.item->claimed, "claiming your pickup leaves the rival's intact");
    }
    require(differentLanes, "bowlers get independent pickup rolls");
    MascotTokens tokens;
    for (const auto &token : tokens.layout) {
        Simulation lane;
        lane.ball = {token.x, token.z + .4f};
        tokens.sweep({token.x, token.z - .4f}, lane);
        tokens.sweep({token.x, token.z - .4f}, lane);
    }
    require(tokens.count == 3 && MascotTokens{}.count == 0, "three unique pickups within one shot");

    Match slugger, umpire;
    umpire.rival = RivalStyle::Umpire;
    const Shot drive = slugger.aiShot(), spareLine = umpire.aiShot();
    require(drive.power > spareLine.power && drive.curve > 0 && spareLine.curve == 0,
            "rivals differ in power and hook at the same difficulty");
    for (bool mascot : {false, true}) {
        int previousScore = -1;
        for (auto difficulty :
             {RivalDifficulty::Rookie, RivalDifficulty::Pro, RivalDifficulty::AllStar,
              RivalDifficulty::HallOfFame}) {
            Match match(mascot);
            match.difficulty = difficulty;
            int rolls = 0;
            while (!match.complete() && rolls++ < 43) {
                int who = match.player;
                auto &lane = match.lanes[who];
                require(lane.standingCount() == match.games[who].standing(),
                        "rack survives turn switching");
                if (who)
                    match.equipRivalPower();
                match.launch(who ? match.aiShot() : Shot{4, .2f, .8f, .1f});
                for (int i = 0; i < int(geometry::shotTimeout * 120 + 240) && lane.active; ++i) {
                    if (mascot && who)
                        match.steering = match.rivalSteering(match.rivalTarget);
                    match.step(1.f / 120);
                }
                require(!lane.active, "shot settles");
                match.settle();
            }
            require(match.complete() && rolls <= 42, "both competitors finish");
            for (const auto &game : match.games)
                require(*game.scores()[9] >= 0 && *game.scores()[9] <= 300, "legal match score");
            int score = *match.games[1].scores()[9];
            std::cout << (mascot ? "Mascot " : "Stadium ") << difficultyName(difficulty) << ": " << score
                      << "\n";
            require(score > previousScore, "higher difficulty scores more in the reference match");
            previousScore = score;
        }
    }
    require(coursePointer(640, 720), "tube itself accepts sling press");
    require(!coursePointer(1160, 260) && !coursePointer(80, 750), "HUD buttons cannot launch shots");
    require(heldPower(.6f) > heldPower(.1f), "timed power meter rises");
    std::cout << "Campaign tests passed: 64 designs, 10 stadiums, save/restore, powers, steering, "
                 "ride pickups, AI matches\n";
}
