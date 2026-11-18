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
    auto drag = alleyDrag(input, 120, 90);
    require(std::abs(drag.power - .65f) < .001f && drag.aim == .5f, "drag sets power and aim");
    require(drag.position == 3 && drag.curve == .2f, "drag locks position and preserves spin");
    require(alleyDrag(input, -1000, 1000).power == 1 && alleyDrag(input, -1000, 1000).aim == -1,
            "drag upper bounds");
    require(alleyDrag(input, 1000, -1000).power == .15f, "downward drag minimum power");
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
    for (bool mascot : {false, true}) {
        Match match;
        match.mascot = mascot;
        int rolls = 0;
        while (!match.complete() && rolls++ < 43) {
            int who = match.player;
            auto &g = match.games[who];
            require(int(match.lanes[who].pins.size()) == g.standing(), "rack survives turn switching");
            match.launch(who ? match.aiShot() : Shot{4, .2f, .8f, .1f});
            for (int i = 0; i < 1000 && match.lanes[who].active; ++i)
                match.step(1.f / 120);
            require(!match.lanes[who].active, "shot settles");
            match.settle();
        }
        require(match.complete() && rolls <= 42, "both competitors finish");
        for (auto &g : match.games)
            require(*g.scores()[9] >= 0 && *g.scores()[9] <= 300, "legal match score");
    }
    std::cout << "Campaign tests passed: 64 designs, 10 stadiums, save/restore, powers, steering, "
                 "boost, AI matches\n";
}
