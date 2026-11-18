#include "campaign.hpp"
#include "alley_controls.hpp"
#include <iostream>
#include <set>
using namespace bowling;
void require(bool b, const char *s) {
    if (!b)
        throw std::runtime_error(s);
}
int main() {
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
    auto pull = mascotPull(input, 100, 80);
    require(std::abs(pull.power - .65f) < .001f && pull.aim == -.5f,
            "sling pulls opposite launch direction");
    require(pull.position == input.position && pull.curve == 0,
            "sling preserves launch position, clears hook");
    require(mascotPull(input, -1000, 1000).power == 1 && mascotPull(input, -1000, 1000).aim == 1,
            "sling limits");
    require(mascotPull(input, 0, -90).power == .15f, "forward pull does not add power");
    Profile p;
    require(p.balls() == 1 && p.stadiums() == 1, "initial unlocks");
    std::set<std::string> names;
    for (int i = 0; i < 64; ++i)
        names.insert(ballName(i));
    require(names.size() == 64, "64 distinct designs");
    require(std::string(arenas[9].name) == "Tropicana Field", "Tropicana finale");
    p.award(100, 150, 0);
    require(p.stadiums() == 1 && p.balls() > 1, "loss gives XP, no stadium");
    for (int arena = 0; arena < 9; ++arena) {
        p.award(180, 100, arena);
        require(p.stadiums() == arena + 2, "ordered stadium unlock");
    }
    p.xp = 5040;
    require(p.balls() == 64, "all balls at 5040 XP");
    auto path = (std::filesystem::temp_directory_path() / "blb-campaign-test.txt").string();
    require(p.save(path), "save");
    Profile restored;
    require(restored.load(path) && restored.xp == p.xp && restored.cleared == p.cleared, "restore");
    p.xp += 80;
    require(p.save(path) && restored.load(path) && restored.xp == p.xp, "save replacement");
    {
        std::ofstream f(path);
        f << "BLB 1 -1 0 0 300 0";
    }
    require(!restored.load(path), "reject corrupt save");
    std::filesystem::remove(path);
    std::filesystem::remove(path + ".bak");
    Match m;
    require(m.equip(Power::Heavy), "equip heavy");
    m.launch({5, 0, .7f, 0});
    require(m.stock[0][0] == 1 && m.lanes[0].ball.r > .8f, "heavy consumes one charge");
    require(!m.equip(Power::Magnet), "no equipping mid-flight");
    Match a, b;
    a.mascot = b.mascot = true;
    a.launch({5, 0, .7f, 0});
    b.launch({5, 0, .7f, 0});
    a.steering = 1;
    for (int i = 0; i < 30; ++i) {
        a.step(1.f / 120);
        b.step(1.f / 120);
    }
    require(a.lanes[0].ball.x > b.lanes[0].ball.x, "mascot steering changes path");
    a.boost();
    float speed = a.lanes[0].ball.vz;
    a.boost();
    require(speed == a.lanes[0].ball.vz && speed > b.lanes[0].ball.vz, "one boost per launch");
    Match lateSteering;
    lateSteering.mascot = true;
    lateSteering.launch({5, 0, .7f, 0});
    lateSteering.lanes[0].ball.z = geometry::headPin - 12;
    lateSteering.steering = 1;
    lateSteering.step(1.f / 120);
    require(lateSteering.lanes[0].ball.vx > 0, "mascot steering remains active near distant pin deck");
    Match bumper;
    bumper.equip(Power::Bumpers);
    bumper.launch({.7f, -1, .7f, 0});
    for (int i = 0; i < 150; ++i)
        bumper.step(1.f / 120);
    require(!bumper.lanes[0].gutter, "bumpers prevent gutter");
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
    for (bool mascot : {false, true}) {
        int previousScore = -1;
        for (auto difficulty :
             {RivalDifficulty::Rookie, RivalDifficulty::Pro, RivalDifficulty::AllStar}) {
            Match match;
            match.mascot = mascot;
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
                    if (mascot && who) {
                        match.steering = match.rivalSteering();
                        if (lane.ball.z >
                            geometry::foulLine + (geometry::headPin - geometry::foulLine) * .4f)
                            match.boost();
                    }
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
                 "boost, AI matches\n";
}
