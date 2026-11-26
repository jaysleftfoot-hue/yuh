#include "alley_controls.hpp"
#include "art.hpp"
#include "audio.hpp"
#include "campaign.hpp"
#include <cstdlib>
#include <iostream>
using namespace bowling;
constexpr int W = 1280, H = 800;
enum class Screen { Title, Lane, Locker, Setup, Scores, Help };
enum class Turn { Ready, Rolling, Reaction, Complete, Replay };
struct RailTile {
    struct Face {
        std::string marks, score;
    };
    Face old, current;
    float phase = 1;

    void update(Face next, float dt) {
        if (next.marks != current.marks || next.score != current.score) {
            old = current;
            current = std::move(next);
            phase = 0;
        }
        phase = std::min(1.f, phase + dt / .39f);
    }
};
// Each stitched rail carries a complete ten-frame scorebook.
void scoreRail(const Game &game, std::array<RailTile, 10> &tiles, float dt, Texture2D rail, bool rivalSide, const char *name, bool atBat, Color inkColor) {
    const float x = rivalSide ? 652 : 4;
    sprite(rail, {x, 0, 624, 77});
    const auto scores = game.scores();
    int total = 0;
    for (int frame = 0; frame < 10; ++frame) {
        const float cx = x + 51 + frame * 48;
        const bool current = atBat && !game.complete && frame == game.frame;
        const Color number = current ? gold : Color{192, 181, 152, 255};
        arcadeCenter(std::to_string(frame + 1), cx, 8, 15, number);
        if (current) {
            DrawCircleV({cx - 14, 15}, 4.5f, {93, 62, 24, 255});
            DrawCircleV({cx - 14, 14}, 2.8f, gold);
        }
        RailTile &tile = tiles[frame];
        tile.update({game.marks(frame), scores[frame] ? std::to_string(*scores[frame]) : ""}, dt);
        const auto &face = tile.phase < .5f ? tile.old : tile.current;
        if (scores[frame]) total = *scores[frame];
        const float hinge = std::abs(std::cos(PI * tile.phase));
        rlPushMatrix();
        rlTranslatef(cx, 40, 0);
        rlScalef(1, std::max(.015f, hinge), 1);
        // The shaded back and the pivot are visible only while a tile turns over.
        if (tile.phase < 1)
            DrawRectangle(-21, -17, 42, 38, Fade({74, 61, 46, 255}, .85f*(1-hinge)));
        if (!face.marks.empty()) arcadeCenter(face.marks, 0, -17, 18, cream);
        else DrawLineEx({-4, -6}, {4, -7}, 2, {106, 112, 102, 235});
        if (!face.score.empty()) arcadeCenter(face.score, 0, 0, 23, inkColor);
        rlPopMatrix();
        // Short scorekeeper's ticks leave the face a single continuous surface.
        DrawLineEx({cx + 25, 25}, {cx + 23, 54}, 1.5f, {142, 137, 116, 128});
    }
    arcadeCenter("TOTAL", x + 556, 10, 14, cream);
    scoreboardLights(std::to_string(total), x + 556, 31, 3.25f, inkColor);
    arcadeCenter(name, x + (rivalSide ? 553 : 68), 73, 20, atBat ? gold : cream);
}
int main(int argc, char **argv) {
    bool smoke = false, startInGame = false, smokeMascot = false, demoShot = false;
    bool demoStrike = false, demoWave = false, splitPreview = false, powerRide = true;
    int captureFrame = 90;
    bool captureAtTick = false;
    int previewArena = -1;
    RivalDifficulty selectedDifficulty = RivalDifficulty::Pro;
    std::string screenshot, scoreboardScreenshot;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--demo-shot")
            demoShot = true;
        if (a == "--demo-wave")
            demoWave = demoShot = true;
        if (a == "--demo-strike")
            demoShot = demoStrike = true;
        if (a == "--preview-split")
            splitPreview = true;
        if (a == "--capture-frame" && i + 1 < argc) {
            captureFrame = std::clamp(std::atoi(argv[++i]), 1, 3600);
            captureAtTick = true;
        }
        if (a == "--smoke")
            smoke = true;
        if (a == "--mascot")
            smokeMascot = true;
        if (a == "--straight-ride")
            powerRide = false;
        if (a == "--difficulty" && i + 1 < argc) {
            std::string difficulty = argv[++i];
            if (difficulty == "rookie")
                selectedDifficulty = RivalDifficulty::Rookie;
            else if (difficulty == "pro")
                selectedDifficulty = RivalDifficulty::Pro;
            else if (difficulty == "all-star")
                selectedDifficulty = RivalDifficulty::AllStar;
            else if (difficulty == "hall-of-fame")
                selectedDifficulty = RivalDifficulty::HallOfFame;
            else {
                std::cerr << "Difficulty must be rookie, pro, all-star, or hall-of-fame\n";
                return 2;
            }
        }
        if (a == "--play")
            startInGame = true;
        if (a == "--arena" && i + 1 < argc)
            previewArena = std::clamp(std::atoi(argv[++i]), 0, 9);
        if (a == "--scoreboard-screenshot" && i + 1 < argc)
            scoreboardScreenshot = argv[++i];
        if (a == "--screenshot" && i + 1 < argc)
            screenshot = argv[++i];
    }
    if (demoWave) {
        startInGame = smokeMascot = powerRide = true;
        previewArena = 9;
    }
    if (splitPreview) {
        previewArena = 1;
        startInGame = true;
        smokeMascot = smoke = false;
    }
    const bool captureRequested = !screenshot.empty() || !scoreboardScreenshot.empty();
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(W, H, "Big League Bowling - Ballpark Tour");
    if (!IsWindowReady())
        return 2;
    SetWindowMinSize(960, 600);
    SetTargetFPS(smoke ? 0 : 60);
    SetExitKey(KEY_NULL);
    uiFont = LoadFontEx(
        (std::string(GetApplicationDirectory()) + "assets/DejaVuSansCondensed-Bold.ttf").c_str(), 96,
        nullptr, 0);
    arcadeFont = LoadFontEx(
        (std::string(GetApplicationDirectory()) + "assets/Bangers-Regular.ttf").c_str(), 96, nullptr, 0);
    GenTextureMipmaps(&arcadeFont.texture);
    SetTextureFilter(arcadeFont.texture, TEXTURE_FILTER_TRILINEAR);
    GenTextureMipmaps(&uiFont.texture);
    SetTextureFilter(uiFont.texture, TEXTURE_FILTER_TRILINEAR);
    RenderTexture2D scene = LoadRenderTexture(W, H);
    Shader antialias =
        LoadShader(nullptr, (std::string(GetApplicationDirectory()) + "assets/antialias.fs").c_str());
    RenderTexture2D replayFeed = LoadRenderTexture(512, 288);
    RenderTexture2D target = LoadRenderTexture(W, H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    Art art;
    art.init();
    art.loadBallSkins();
    art.loadBallparkArt();
    art.makeSigns();
    Texture2D titleArt =
        LoadTexture((std::string(GetApplicationDirectory()) + "assets/title-screen.png").c_str());
    if (!titleArt.id)
        throw std::runtime_error("Missing title-screen.png beside executable");
    GenTextureMipmaps(&titleArt);
    SetTextureFilter(titleArt, TEXTURE_FILTER_TRILINEAR);
    Texture2D portraits =
        LoadTexture((std::string(GetApplicationDirectory()) + "assets/ballpark-roster.jpg").c_str());
    if (!portraits.id)
        throw std::runtime_error("Missing ballpark-roster.jpg");
    GenTextureMipmaps(&portraits);
    SetTextureFilter(portraits, TEXTURE_FILTER_TRILINEAR);
    auto uiTexture = [](const char *name) {
        Texture2D t = LoadTexture((std::string(GetApplicationDirectory()) + "assets/" + name).c_str());
        if (!t.id)
            throw std::runtime_error(std::string("Missing UI asset: ") + name);
        GenTextureMipmaps(&t);
        SetTextureFilter(t, TEXTURE_FILTER_TRILINEAR);
        return t;
    };
    uiBallShader =
        LoadShader(nullptr, (std::string(GetApplicationDirectory()) + "assets/ui-ball.fs").c_str());
    const int lockerLongitude = GetShaderLocation(uiBallShader, "longitude");
    float previewAngle = 0;
    uiSlate = uiTexture("arcade-slate.png");
    uiFrame = uiTexture("arcade-frame.png");
    uiButton = uiTexture("arcade-button.png");
    Audio audio;
    audio.init();
    const char *dir = std::getenv("APPDATA");
    if (!dir)
        dir = std::getenv("XDG_DATA_HOME");
    if (!dir)
        dir = std::getenv("HOME");
    std::string save = std::string(dir ? dir : ".") + "/big-league-bowling-profile.txt";
    Profile profile;
    if (!profile.load(save))
        profile.load(save + ".bak");
    Match match(smokeMascot);
    Shot shot;
    StadiumSetup stadiumSetup;
    PinReplay replay;
    MascotMotion mascotMotion;
    std::array<std::array<RailTile, 10>, 2> railTiles{};
    float strikeSeconds = 0;
    bool spareNext = false;
    MascotTokens tokens;
    RidePickups ridePickups;
    std::mt19937 rideRng(smoke || captureRequested ? 628u : std::random_device{}());
    float mouseSteering = 0;
    std::array<PowerPickups, 2> pickups;
    StadiumCue stadiumCue = StadiumCue::None;
    float cueSeconds = 0, pickupSeconds = 0;
    std::string pickupMessage, challengeMessage, rivalLine;
    bool splitDrill = splitPreview;
    std::mt19937 pickupSeedSource(smoke || captureRequested ? 271u : std::random_device{}());
    Turn turn = Turn::Ready;
    Screen screen = startInGame || smoke ? Screen::Lane : Screen::Title;
    Screen lockerReturn = Screen::Title;
    int arena = previewArena < 0 ? 0 : previewArena;
    std::mt19937 rosterRng(smoke || captureRequested ? 391u : std::random_device{}());
    int opponent = drawHomeRival(arena, rosterRng);
    int &skin = profile.equippedBall;
    int rolls = 0, ticks = 0, selectedSkin = skin;
    float time = 0, accum = 0, reaction = 0, chargeTime = 0, aiWait = .9f, celebration = 0;
    Vector2 dragStart{};
    bool mouseCharge = false, stadiumArmed = false;
    bool equipmentOpen = false, avatarSaveFailed = false, lockerSaveFailed = false;
    bool paused = false, charge = false, done = false, ok = false, awarded = false;
    std::string result = "WELCOME TO THE TOUR", reward;
    Camera3D camera{{0, 3.0f, -6.2f}, {0, 1.7f, 23}, {0, 1, 0}, 43, CAMERA_PERSPECTIVE};
    auto resetShot = [&]() {
        strikeSeconds = 0;
        spareNext = false;
        shot = Shot{};
        stadiumSetup.reset();
        ridePickups = {};
        mouseSteering = 0;
        stadiumArmed = false;
    };
    auto saveCareer = [&]() {
        return smoke || captureRequested || previewArena >= 0 || profile.save(save);
    };
    auto completeChallenge = [&](Challenge challenge) {
        if (!profile.finish(challenge))
            return;
        for (const auto &item : challenges)
            if (item.challenge == challenge)
                challengeMessage = "CHALLENGE COMPLETE | " + ballName(item.ball) + " UNLOCKED";
        if (!saveCareer())
            challengeMessage += " | SAVE FAILED";
    };
    auto preparePickups = [&]() {
        tokens = {};
        if (!splitDrill && !match.mascot)
            pickups[match.player].enterFrame(match.games[match.player].frame);
        pickupSeconds = 0;
    };
    auto restart = [&]() {
        railTiles = {};
        match = Match(match.mascot);
        match.stage = arena;
        match.rival = roster[opponent].style;
        if (splitDrill)
            setSevenTenRack(match.lanes[0]);
        for (auto &bowlerPickups : pickups)
            bowlerPickups.startMatch(selectedDifficulty, pickupSeedSource());
        equipmentOpen = false;
        preparePickups();
        mascotMotion = {};
        replay.reset();
        stadiumCue = StadiumCue::None;
        cueSeconds = 0;
        challengeMessage.clear();
        rivalLine.clear();
        match.difficulty = selectedDifficulty;
        rolls = 0;
        celebration = accum = 0;
        turn = Turn::Ready;
        charge = false;
        paused = false;
        awarded = false;
        reward.clear();
        resetShot();
        aiWait = .9f;
        result = "MAKE SOME NOISE!";
    };
    restart();
    int lastLoggedScreen = -1;
    auto launch = [&]() {
        if (turn == Turn::Ready && !paused) {
            replay.reset();
            if (match.mascot) {
                shot.aim = shot.curve = 0;
                if (powerRide)
                    ridePickups.reveal(rideRng);
                mascotMotion.release();
                if (!screenshot.empty())
                    std::cout << "Mascot launch: position=" << shot.position << " aim=" << shot.aim
                              << " power=" << shot.power << " pickups=" << bool(ridePickups.route)
                              << std::endl;
            }
            match.launch(shot);
            rivalLine.clear();
            turn = Turn::Rolling;
            charge = false;
            audio.play(audio.bowl);
        }
    };
    auto capture = [&]() {
        if (!scoreboardScreenshot.empty()) {
            Image board = LoadImageFromTexture(art.scoreboard.texture);
            ImageFlipVertical(&board);
            ExportImage(board, scoreboardScreenshot.c_str());
            UnloadImage(board);
        }
        if (!screenshot.empty()) {
            Image image = LoadImageFromScreen();
            ExportImage(image, screenshot.c_str());
            UnloadImage(image);
        }
    };
    while (!WindowShouldClose() && !done) {
        float dt = smoke ? 1.f / 15 : captureRequested ? 1.f / 60 : std::min(GetFrameTime(), .25f);
        time += dt;
        ++ticks;
        if (demoShot && ticks == 30 && screen == Screen::Lane) {
            if (splitDrill)
                shot = {2.f, 0.f, 1.f, 0.f};
            else if (demoStrike)
                shot = {4.5f, 0.f, .72f, 0.f};
            launch();
        }
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        float scale = std::min(GetScreenWidth() / float(W), GetScreenHeight() / float(H));
        Vector2 offset{(GetScreenWidth() - W * scale) / 2, (GetScreenHeight() - H * scale) / 2};
        SetMouseOffset(-int(offset.x), -int(offset.y));
        SetMouseScale(1 / scale, 1 / scale);
        if (IsKeyPressed(KEY_F11))
            ToggleFullscreen();
        if (IsKeyPressed(KEY_M))
            audio.toggle();
        const bool canBrowse = screen == Screen::Lane && match.player == 0 && turn == Turn::Ready &&
                               !splitDrill && !match.mascot && !paused;
        if (canBrowse && IsKeyPressed(KEY_P)) {
            equipmentOpen = !equipmentOpen;
            charge = stadiumArmed = false;
        }
        if (equipmentOpen && IsKeyPressed(KEY_ESCAPE))
            equipmentOpen = false;
        else if (IsKeyPressed(KEY_ESCAPE)) {
            if (screen == Screen::Lane) {
                paused = !paused;
                charge = false;
                stadiumArmed = false;
            } else if (screen == Screen::Locker)
                screen = lockerReturn;
            else if (screen != Screen::Title)
                screen = Screen::Title;
            else
                done = true;
        }
        bool human = match.player == 0;
        auto &sim = match.lanes[match.player];
        if (screen == Screen::Lane && !paused && !equipmentOpen) {
            if (turn == Turn::Ready) {
                if (!human) {
                    aiWait -= dt;
                    if (aiWait <= 0) {
                        shot = match.aiShot();
                        if (!match.mascot)
                            match.equipRivalPower();
                        launch();
                    }
                } else if (smoke) {
                    const Shot shots[] = {
                        {1, -1, .7f, 0}, {5, 0, .72f, 0}, {4, .3f, .65f, -.2f}, {7, -.2f, .9f, .3f}};
                    shot = shots[rolls % 4];
                    if (rolls % 3 == 0)
                        match.equip(Power(1 + rolls % 3));
                    launch();
                } else {
                    bool alleyPointer = coursePointer(GetMouseX(), GetMouseY());
                    if (!match.mascot) {
                        float position =
                            shot.position + (float(IsKeyDown(KEY_D)) - float(IsKeyDown(KEY_A))) * dt * 3;
                        float curve = shot.curve + (float(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_E)) -
                                                    float(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_Q))) *
                                                       dt;
                        if (alleyPointer && GetMouseDelta().x != 0) {
                            position = alleyPosition(GetMouseX());
                            curve = std::clamp((GetMouseX() - 640.f) / 350.f, -1.f, 1.f);
                        }
                        if (IsKeyPressed(KEY_BACKSPACE))
                            stadiumSetup.back();
                        stadiumSetup.adjust(shot, position, curve);
                        stadiumSetup.tick(shot, dt);
                        // Confirm on release, so a click cannot carry into the next phase.
                        if (!stadiumArmed) {
                            stadiumArmed = !IsKeyDown(KEY_SPACE) && !IsKeyDown(KEY_ENTER) &&
                                           !IsMouseButtonDown(MOUSE_BUTTON_LEFT);
                        } else if (IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_ENTER) ||
                                   (alleyPointer && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))) {
                            if (stadiumSetup.confirm())
                                launch();
                            else
                                audio.play(audio.click);
                            if (!screenshot.empty())
                                std::cout << "Shot phase: " << int(stadiumSetup.phase)
                                          << " state=" << int(turn) << " position=" << shot.position
                                          << " curve=" << shot.curve << " power=" << shot.power
                                          << std::endl;
                        }
                    } else {
                        if (IsKeyPressed(KEY_BACKSPACE))
                            charge = false;
                        if (!charge) {
                            const float direction = float(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) -
                                                    float(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
                            shot.position = std::clamp(shot.position + direction * dt * 4,
                                                       geometry::launchMin, geometry::launchMax);
                            if (alleyPointer && GetMouseDelta().x != 0)
                                shot.position = alleyPosition(GetMouseX());
                            mouseCharge = alleyPointer && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                            charge = mouseCharge || IsKeyPressed(KEY_SPACE);
                            if (charge) {
                                dragStart = GetMousePosition();
                                chargeTime = 0;
                            }
                        }
                        if (charge) {
                            chargeTime += dt;
                            const float mouseY = GetMouseY();
                            const float pull = mouseCharge ? mascotMouseDraw(dragStart.y, mouseY, H - 2.f)
                                                          : chargeTime * 155;
                            const bool drawn = pull >= 10.f && (!mouseCharge || mouseY - dragStart.y >= 8.f);
                            shot = mascotPull(shot, pull);
                            mascotMotion.aim((shot.power - .15f) / .85f);
                            if (powerRide && drawn)
                                ridePickups.reveal(rideRng);
                            if (mouseCharge ? IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
                                            : IsKeyReleased(KEY_SPACE)) {
                                if (drawn)
                                    launch();
                                else
                                    charge = false;
                            }
                        } else
                            mascotMotion.aim(0);
                    }
                    if (!splitDrill && !match.mascot)
                        for (int i = 0; i < powerCount; ++i)
                            if (IsKeyPressed(KEY_ONE + i))
                                match.equip(Power(i + 1));
                }
            }
            if (turn == Turn::Rolling && match.mascot) {
                if (!human) {
                    float target = match.rivalTarget;
                    if (ridePickups.route && selectedDifficulty != RivalDifficulty::Rookie) {
                        for (const auto &item : *ridePickups.route) {
                            const bool pursue = selectedDifficulty != RivalDifficulty::Pro ||
                                                item.power == RidePower::Double;
                            if (pursue && item.position.z > sim.ball.z) {
                                target = item.position.x;
                                break;
                            }
                        }
                    }
                    match.steering = match.rivalSteering(target);
                } else if (smoke || (demoWave && rolls == 0)) {
                    float target = demoWave ? 4.5f : geometry::halfWidth + std::sin(rolls * 2.37f) * 1.6f;
                    if (ridePickups.route)
                        for (const auto &item : *ridePickups.route)
                            if (item.position.z > sim.ball.z && (!demoWave || item.power == RidePower::Wave)) {
                                target = item.position.x;
                                break;
                            }
                    match.steering = std::clamp((target - sim.ball.x) * .65f - sim.ball.vx * .35f, -1.f, 1.f);
                } else {
                    const float keys = float(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                                       float(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A));
                    // Motion made while drawing the sling ends at release.
                    const float delta = sim.elapsed > 0 ? GetMouseDelta().x : 0.f;
                    mouseSteering = std::clamp(mouseSteering * std::exp(-10.f * dt) + delta / 28.f, -1.f, 1.f);
                    match.steering = keys != 0 ? keys : mouseSteering;
                }
            }
            accum += dt;
            while (turn == Turn::Rolling && accum >= 1.f / 120) {
                constexpr float step = 1.f / 120;
                const Body before = sim.ball;
                const int impacts = sim.impacts;
                const bool finished = match.step(step);
                if (match.mascot) {
                    const auto picked = ridePickups.sweep(before, sim);
                    if (picked != RidePower::None) {
                        pickupMessage = picked == RidePower::Wave ? "WAVE CRASH ARMED" : "2X PINS ARMED";
                        pickupSeconds = 1.2f;
                        audio.play(audio.click);
                    }
                    if (ridePickups.impact(sim, impacts)) {
                        pickupMessage = "WAVE CRASH!";
                        pickupSeconds = 1.f;
                    }
                    mascotMotion.update(step, sim);
                    if (human && !profile.completed(Challenge::MascotPickups)) {
                        tokens.sweep(before, sim);
                        if (tokens.count == int(tokens.layout.size()))
                            completeChallenge(Challenge::MascotPickups);
                    }
                }
                if (!match.mascot) {
                    const Power picked = pickups[match.player].sweep(before, match);
                    if (picked != Power::None && human) {
                        pickupMessage = std::string(powerName(picked)) + " PICKED UP";
                        pickupSeconds = 1.6f;
                    }
                }
                if (!smoke)
                    replay.record(sim, finished, mascotMotion.pose);
                if (sim.impacts > impacts)
                    audio.play(audio.hit);
                accum -= step;
                if (!finished)
                    continue;

                const auto roll = match.outcome();
                if (!match.mascot && !splitDrill) {
                    Game next = match.games[match.player];
                    next.roll(roll.knocked);
                    spareNext = !next.complete && next.frame == match.games[match.player].frame && !next.freshRack();
                }
                const int bonus = ridePickups.bonus(roll.knocked);
                match.rideBonus[match.player] += bonus;
                if (bonus > 0) {
                    pickupMessage = "+" + std::to_string(bonus) + " RIDE BONUS";
                    pickupSeconds = 2.5f;
                }
                const auto cue = strikeCue(arena, match.mascot, match.strikeStreak[match.player], roll);
                if (cue != StadiumCue::None) {
                    stadiumCue = cue;
                    cueSeconds = 4.f;
                }
                result = cue == StadiumCue::Turkey ? "TURKEY! RAYS UP!"
                         : roll.strike ? "STRIKE! OUT OF THE PARK!"
                         : roll.split ? "7-10 SPLIT CONVERTED!"
                         : roll.spare ? "SPARE!"
                         : roll.knocked == 0 ? "JUST A BIT OUTSIDE!"
                                             : std::to_string(roll.knocked) + " PINS DOWN!";
                if (human && !match.mascot && arena == 1 && roll.split)
                    completeChallenge(Challenge::FenwaySplit);
                if (!splitDrill)
                    rivalLine = rivalReaction(match.rival, roll, human);
                if (roll.knocked >= 7 || roll.split) {
                    celebration = 2;
                    audio.play(audio.cheer);
                }
                if (replay.available(roll))
                    turn = Turn::Replay;
                else {
                    turn = Turn::Reaction;
                    reaction = smoke ? .1f : PinsetterCycle::duration;
                }
            }
            if (turn != Turn::Rolling)
                accum = 0;
            if (turn == Turn::Replay) {
                replay.seconds += dt;
                const Rectangle replaySkip{500, 716, 280, 58};
                if (replay.finished() || IsKeyReleased(KEY_SPACE) || IsKeyReleased(KEY_ENTER) ||
                    (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) &&
                     CheckCollisionPointRec(GetMousePosition(), replaySkip))) {
                    turn = Turn::Reaction;
                    reaction = PinsetterCycle::duration;
                }
            }
            if (turn == Turn::Reaction) {
                reaction -= dt;
                if (reaction <= 0) {
                    ++rolls;
                    if (splitDrill) {
                        turn = Turn::Complete;
                    } else {
                        match.settle();
                        turn = match.complete() ? Turn::Complete : Turn::Ready;
                        aiWait = smoke ? 0 : .9f;
                        resetShot();
                        mascotMotion = {};
                        preparePickups();
                    }
                }
            }
            if (turn == Turn::Complete && !awarded) {
                if (!splitDrill) {
                    int yours = scoreTotal(match.games[0]), theirs = scoreTotal(match.games[1]);
                    const int finalScore = match.total(0), rivalScore = match.total(1);
                    result = finalScore > rivalScore    ? "YOU WIN!"
                             : finalScore == rivalScore ? "A TIE! REMATCH?"
                                               : "RIVAL WINS - KEEP SWINGING!";
                    int old = profile.balls(), oldA = profile.stadiums();
                    if (match.mascot) {
                        int &record = powerRide ? profile.bestPowerRide : profile.bestStraightRide;
                        record = std::max(record, finalScore);
                    }
                    int xp = profile.award(yours, theirs, arena, match.rideBonus[0], match.rideBonus[1]);
                    if (!match.mascot && arena == 0 && yours > theirs)
                        completeChallenge(Challenge::WrigleyWin);
                    reward = "+" + std::to_string(xp) + " XP | " + std::to_string(profile.balls()) +
                             " / 64 BALLS";
                    if (profile.balls() > old)
                        reward += " | NEW BALLS!";
                    if (profile.stadiums() > oldA) {
                        if (profile.stadiums() == 10) {
                            stadiumCue = StadiumCue::TropicanaUnlocked;
                            cueSeconds = 5.f;
                        } else
                            reward += " | NEXT STADIUM OPEN!";
                    }
                    if (closeFinish(finalScore, rivalScore)) {
                        constexpr const char *closeLines[] = {"THAT WENT RIGHT TO THE WIRE.",
                                                               "CLOSE CALL. LET'S GO AGAIN.",
                                                               "I WAS HOLDING MY BREATH!"};
                        rivalLine = closeLines[int(match.rival)];
                    } else
                        rivalLine.clear();
                    if (!saveCareer())
                        reward += " | SAVE FAILED";
                } else {
                    reward = match.outcome().split ? "FENWAY CHALLENGE COMPLETE" : "TRY A FIRM SHOT ON THE INSIDE EDGE";
                }
                awarded = true;
            }
            pickupSeconds = std::max(0.f, pickupSeconds - dt);
            if (turn != Turn::Replay)
                cueSeconds = std::max(0.f, cueSeconds - dt);
            if (cueSeconds == 0 && stadiumCue == StadiumCue::Turkey)
                stadiumCue = StadiumCue::None;
        }
        audio.update();
        celebration = std::max(0.f, celebration - dt);
        const bool previewTick = captureAtTick && ticks == captureFrame;
        if (smoke && ticks % 12 && !match.complete() && !previewTick)
            continue;
        art.arena = arena;
        art.skin = skin;
        art.mascot = match.mascot;
        const auto &pickup = pickups[match.player].item;
        art.powerPickup = !splitDrill && !match.mascot && pickup ? &*pickup : nullptr;
        art.ridePickups = match.mascot && (charge || turn == Turn::Rolling) ? &ridePickups : nullptr;
        art.mascotTokens = match.mascot && match.player == 0 &&
                           !profile.completed(Challenge::MascotPickups) ? &tokens : nullptr;
        art.cue = screen == Screen::Lane ? stadiumCue : StadiumCue::None;
        art.cueSeconds = cueSeconds;
        const bool replaying = screen == Screen::Lane && turn == Turn::Replay;
        const auto &view = replaying ? replay.view() : match.lanes[match.player];
        const bool strike = screen == Screen::Lane && turn != Turn::Ready && turn != Turn::Complete &&
                            match.outcome().strike && view.knocked() == 10;
        if (!strike) strikeSeconds = 0;
        else if (!paused) strikeSeconds += dt;
        art.setter.progress = turn == Turn::Reaction
            ? std::clamp(1.f - reaction / (smoke ? .1f : PinsetterCycle::duration), 0.f, 1.f) : 0;
        art.returningBall = spareNext && art.setter.progress >= .63f;
        art.strikeSalute = strike ? art.setter.progress : 0;
        art.crowdCheer = PinsetterCycle::ease(strikeSeconds, 0, .26f);
        art.mascotPose = replaying ? replay.frame().rider : mascotMotion.pose;
        art.wave = ridePickups.wave;
        if (replaying) {
            // Use a deck-only render for the stadium board to avoid video feedback.
            const float rack = replay.view().layout.headPin();
            const Camera3D boardCamera{{-7, 6, rack - 10}, {0, 1, rack + 2},
                                       {0, 1, 0}, 44, CAMERA_PERSPECTIVE};
            BeginTextureMode(replayFeed);
            ClearBackground({12, 30, 39, 255});
            art.world(replay.view(), shot, false, arenas[arena].team, boardCamera, replay.seconds, true);
            EndTextureMode();
        }
        art.updateScoreboard(match, paused ? 0.f : dt, replaying ? &replayFeed.texture : nullptr);
        const auto cameraStyle = match.mascot ? geometry::mascotCamera : geometry::stadiumCamera;
        camera.fovy = cameraStyle.fov;
        const float rack = view.layout.headPin();
        Vector3 desired{geometry::renderX(shot.position) * .25f, cameraStyle.height, cameraStyle.back},
            look{0, cameraStyle.targetHeight, rack};
        if (screen != Screen::Lane) {
            desired = match.mascot ? Vector3{-3.2f, 4.5f, -5} : Vector3{-9, 7, -4};
            look = {0, 1, match.mascot ? 13.f : 19.f};
        } else if (turn == Turn::Rolling || turn == Turn::Reaction) {
            float z = std::min(view.ball.z, rack + 1);
            desired = {std::clamp(geometry::renderX(view.ball.x) * .65f, -3.f, 3.f),
                       match.mascot ? 3.1f : 2.0f, z - (match.mascot ? 5.3f : 5.8f)};
            look = {std::clamp(geometry::renderX(view.ball.x) * .3f, -1.5f, 1.5f),
                    match.mascot ? 1.5f : .9f, z + 12};
        }
        if (replaying) {
            desired = {-9, 5.4f, rack - 13};
            look = {1.f, 3.f, rack + 7};
        }
        if (art.returningBall) {
            desired = {-4.7f, 2.62f, 3.20f};
            look = {-7.65f, 1.40f, 7.65f};
        }
        if (screen == Screen::Lane && (turn == Turn::Ready || replaying || art.returningBall)) {
            camera.position = desired;
            camera.target = look;
        }
        camera.position =
            Vector3Lerp(camera.position, desired, std::min(1.f, dt * (turn == Turn::Rolling ? 18 : 5)));
        camera.target =
            Vector3Lerp(camera.target, look, std::min(1.f, dt * (turn == Turn::Rolling ? 18 : 5)));
        BeginTextureMode(scene);
        ClearBackground({45, 92, 129, 255});
        DrawRectangleGradientV(0, 0, W, 550, {17, 39, 75, 255}, {140, 182, 193, 255});
        if (replaying && match.mascot)
            DrawTexturePro(art.tank, {0, 0, float(art.tank.width), float(art.tank.height)},
                           {0, 0, float(W), float(H)}, {}, 0, {72, 122, 145, 255});
        if (screen == Screen::Lane || screen == Screen::Setup)
            art.world(view, shot, turn == Turn::Ready && screen == Screen::Lane, arenas[arena].team,
                      camera, replaying ? replay.seconds : time);
        EndTextureMode();
        BeginTextureMode(target);
        ClearBackground(BLACK);
        BeginShaderMode(antialias);
        DrawTexturePro(scene.texture, {0, 0, float(W), -float(H)}, {0, 0, float(W), float(H)}, {0, 0}, 0,
                       WHITE);
        EndShaderMode();
        if (screen != Screen::Lane) {
            DrawTexturePro(titleArt, {0, 0, float(titleArt.width), float(titleArt.height)},
                           {0, 0, float(W), float(H)}, {0, 0}, 0, WHITE);
        }
        uiBlocked = paused || equipmentOpen;
        if (screen == Screen::Title) {
            title("WELCOME TO THE BIG LEAGUES!", 565, 16, 20, cream);
            if (button({282, 650, 320, 52}, "SINGLE PLAYER", true) || IsKeyPressed(KEY_ENTER)) {
                match = Match(false);
                opponent = drawHomeRival(arena, rosterRng);
                screen = Screen::Setup;
                audio.play(audio.click);
            }
            if (button({660, 650, 320, 52}, "MASCOT ADVENTURE", true)) {
                match = Match(true);
                opponent = drawHomeRival(arena, rosterRng);
                screen = Screen::Setup;
                audio.play(audio.click);
            }
            if (button({145, 718, 190, 42}, "BALL LOCKER")) {
                selectedSkin = skin;
                previewAngle = 0;
                lockerSaveFailed = false;
                lockerReturn = Screen::Title;
                screen = Screen::Locker;
            }
            if (button({350, 718, 145, 42}, "SCORES"))
                screen = Screen::Scores;
            if (button({515, 718, 215, 42}, audio.muted ? "SOUND OFF" : "SOUND ON"))
                audio.toggle();
            if (button({750, 718, 140, 42}, "HELP"))
                screen = Screen::Help;
            if (button({920, 718, 140, 42}, "QUIT"))
                done = true;
        } else if (screen == Screen::Scores || screen == Screen::Help) {
            modal({184, 35, 912, 730}, screen == Screen::Scores ? "HALL OF FAME" : "HOW TO BOWL");
            if (screen == Screen::Scores) {
                portraitCard(portraits, profile.avatar, {290, 242, 200, 270}, "THE BIG LEAGUER");
                arcadeCenter(std::to_string(profile.wins) + " WINS  /  " +
                                 std::to_string(profile.matches) + " GAMES",
                             770, 267, 32);
                arcadeCenter("BEST GAME  " + std::to_string(profile.best), 770, 315, 31, gold);
                arcadeCenter("STRAIGHT RIDE  " + std::to_string(profile.bestStraightRide) +
                             "  /  POWER RIDE  " + std::to_string(profile.bestPowerRide), 770, 354, 21, aqua);
                arcadeCenter(std::to_string(profile.xp) + " CAREER XP", 770, 388, 29);
                arcadeCenter(std::to_string(profile.balls()) + " / 64 BALLS UNLOCKED", 770, 440, 26);
                arcadeCenter(std::to_string(profile.stadiums()) + " / 10 BALLPARKS", 770, 487, 26);
                arcadeCenter("FINAL DESTINATION: TROPICANA FIELD", 640, 574, 26, gold);
            } else {
                arcadeCenter("STADIUM: THREE STEPS TO A STRIKE", 640, 210, 32, gold);
                arcadeCenter("1. SPOT: MOVE THE BALL, CLICK TO LOCK", 640, 260, 26);
                arcadeCenter("2. CURVE: MOVE LEFT / RIGHT, CLICK TO LOCK", 640, 304, 26);
                arcadeCenter("3. POWER: CLICK WHEN THE METER IS RIGHT", 640, 348, 26);
                center("A / D: spot   |   Arrows: curve   |   Space: confirm   |   Backspace: go back",
                       640, 394, 16, cream);
                arcadeCenter("MASCOT: GRAB THE TUBE AND PULL BACK", 640, 450, 30, gold);
                arcadeCenter("PULL STRAIGHT BACK. RELEASE TO LAUNCH.", 640, 493, 25);
                arcadeCenter("MOUSE OR A / D: SPOT THE TUBE, THEN STEER", 640, 533, 24);
                center("Power Ride coins affect this throw. In Stadium, use 1–6 to equip.",
                       640, 571, 16, cream);
                center("Fastball: faster shot   |   Changeup: smaller rival ball   |   Wild Pitch: rival drift",
                       640, 596, 14, cream);
                center("Space: skip replay   |   Escape: pause   |   M: sound   |   F11: fullscreen",
                       640, 619, 16, cream);
            }
            if (plaqueButton({516, 650, 248, 64}, "BACK"))
                screen = Screen::Title;
        } else if (screen == Screen::Setup) {
            modal({180, 24, 920, 752}, "BALLPARK SHOWDOWN");
            // Compact versus composition: park preview above two portrait plaques.
            panel({486, 154, 308, 180});
            DrawTexturePro(scene.texture, {0, 0, float(W), -float(H)}, {502, 172, 276, 143}, {0, 0}, 0,
                           WHITE);
            int parkStep = 0;
            if (button({425, 211, 54, 58}, "<")) parkStep = -1;
            if (button({801, 211, 54, 58}, ">")) parkStep = 1;
            if (parkStep) {
                arena = (arena + parkStep + 10) % 10;
                opponent = drawHomeRival(arena, rosterRng);
            }
            int step = 0;
            if (button({225, 430, 48, 66}, "<") || IsKeyPressed(KEY_LEFT)) step = -1;
            if (button({489, 430, 43, 66}, ">") || IsKeyPressed(KEY_RIGHT)) step = 1;
            if (step) {
                profile.avatar = (profile.avatar + step + 6) % 6;
                avatarSaveFailed = !saveCareer();
                audio.play(audio.click);
            }
            arcadeCenter(arenas[arena].name, 640, 341, 28);
            portraitCard(portraits, profile.avatar, {275, 343, 212, 246}, "PLAYER ONE");
            portraitCard(portraits, opponent, {793, 343, 212, 246}, "HOME RIVAL");
            arcadeCenter("VS", 640, 387, 76, {170, 255, 64, 255});
            if (plaqueButton({495, 487, 290, 68}, match.mascot ? "MASCOT RIDE" : "STADIUM BOWLING"))
                match = Match(!match.mascot);
            if (button({495, 554, 290, 40}, std::string("AI: ") + difficultyName(selectedDifficulty))) {
                selectedDifficulty = static_cast<RivalDifficulty>((int(selectedDifficulty) + 1) % rivalDifficultyCount);
                audio.play(audio.click);
            }
            bool unlocked = arena < profile.stadiums();
            std::string hint = match.mascot ? "PULL BACK. LET FLY. RIDE THE RAY!"
                                            : "SPOT. CURVE. POWER. TAKE ON THE RIVAL!";
            if (!unlocked)
                hint = "WIN AT " + std::string(arenas[arena - 1].name) + " TO UNLOCK";
            else if (match.mascot && !profile.completed(Challenge::MascotPickups))
                hint = "THREE TOKENS IN ONE RIDE EARNS TOUCH TANK";
            else if (!match.mascot && arena == 0 && !profile.completed(Challenge::WrigleyWin))
                hint = "BEAT THE HOME RIVAL TO EARN IVY LEAGUE CUB";
            arcadeCenter(avatarSaveFailed ? "AVATAR SAVE FAILED" : hint, 640, 608, 24, unlocked ? cream : gold);
            if (plaqueButton({395, 650, 226, 65}, unlocked ? "START" : "LOCKED", unlocked) ||
                (IsKeyPressed(KEY_ENTER) && unlocked)) {
                screen = Screen::Lane;
                splitDrill = false;
                restart();
                audio.play(audio.cheer);
            }
            if (plaqueButton({659, 650, 226, 65}, "BACK"))
                screen = Screen::Title;
            if (button({275, 594, 212, 35}, "BALL LOCKER")) {
                selectedSkin = skin;
                previewAngle = 0;
                lockerSaveFailed = false;
                lockerReturn = Screen::Setup;
                screen = Screen::Locker;
            }
            if (arena == 1 && unlocked && !match.mascot) {
                if (button({808, 591, 184, 37}, "7-10 SPLIT DRILL")) {
                    splitDrill = true;
                    screen = Screen::Lane;
                    restart();
                }
            } else if (match.mascot) {
                if (button({789, 585, 222, 39}, powerRide ? "POWER RIDE" : "STRAIGHT RIDE")) {
                    powerRide = !powerRide;
                    audio.play(audio.click);
                }
            } else
                arcadeCenter("PARK " + std::to_string(arena + 1) + " / 10", 900, 598, 20, gold);
        } else if (screen == Screen::Locker) {
            sprite(art.shop, {0, 0, W, H});
            arcadeCenter("THE BALL LOCKER", 446, 103, 52, cream);
            arcadeCenter("30 CLUBS  /  30 FUN DESIGNS  /  4 RAYS SPECIALS", 446, 158, 22);
            // Keep the career slot order on the felt sample board.
            for (int id = 0; id < 64; ++id) {
                int col = id % 8, row = id / 8;
                Rectangle cell{119 + col * 84.f, 193 + row * 58.f, 78, 55};
                bool unlocked = profile.ballUnlocked(id);
                if (id == selectedSkin)
                    DrawRing({cell.x + 39, cell.y + 23}, 24, 27, 0, 360, 40, gold);
                ballIcon(art.skins[id], cell.x + 16, cell.y, 46, unlocked);
                arcadeCenter(std::to_string(id + 1) + (unlocked ? "" : " X"), cell.x + 39, cell.y + 43,
                             13, unlocked ? cream : GRAY);
                if (CheckCollisionPointRec(GetMousePosition(), cell) &&
                    IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    selectedSkin = id;
                    previewAngle = 0;
                }
            }
            arcadeCenter("BALL " + std::to_string(selectedSkin + 1), 1014, 123, 31, gold);
            arcadeCenter("ARROW KEYS TO TURN", 1014, 163, 15);
            previewAngle = std::fmod(previewAngle + .034f*dt + (float(IsKeyDown(KEY_RIGHT)) -
                                                      float(IsKeyDown(KEY_LEFT))) * dt * .32f, 1.f);
            SetShaderValue(uiBallShader, lockerLongitude, &previewAngle, SHADER_UNIFORM_FLOAT);
            ballIcon(art.skins[selectedSkin], 924, 210, 180);
            const float front = 0;
            SetShaderValue(uiBallShader, lockerLongitude, &front, SHADER_UNIFORM_FLOAT);
            std::string name = ballName(selectedSkin);
            center(name, 1014, 471, std::min(24.f, 278.f * 24 / tw(name,24)), {53, 36, 19, 255});
            center(profile.ballUnlocked(selectedSkin)
                             ? "UNLOCKED"
                             : std::to_string(selectedSkin * 80) + " CAREER XP",
                         1014, 506, 17, ink);
            if (const auto *challenge = ballChallenge(selectedSkin); challenge && !profile.ballUnlocked(selectedSkin)) {
                const std::string hint = std::string("OR ") + challenge->instruction;
                center(hint, 1014, 546, std::min(13.f, 235.f * 13 / tw(hint, 13)), cream);
            }
            if (plaqueButton({829, 722, 187, 55}, skin == selectedSkin ? "EQUIPPED" : "EQUIP",
                             profile.ballUnlocked(selectedSkin))) {
                skin = selectedSkin;
                lockerSaveFailed = !saveCareer();
                audio.play(audio.click);
            }
            constexpr std::array<int, 6> raysShelf{26, 56, 60, 61, 62, 63};
            arcadeCenter("TAMPA BAY COLLECTION", 1014, 579, 22, gold);
            for (size_t slot = 0; slot < raysShelf.size(); ++slot) {
                float x = 847.f + slot * 57;
                int id = raysShelf[slot];
                ballIcon(art.skins[id], x, 625, 47, profile.ballUnlocked(id));
                if (CheckCollisionPointRec(GetMousePosition(), {x-4, 619, 53, 60}) &&
                    IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    selectedSkin = id;
                    previewAngle = 0;
                }
            }
            arcadeCenter(lockerSaveFailed ? "SAVE FAILED" : "EQUIPPED: " + ballName(skin), 445, 704, 22, gold);
            if (plaqueButton({1028, 722, 164, 55}, "BACK"))
                screen = lockerReturn;
        } else {
            baseballHud(portraits, art.pinGauge, profile.avatar, opponent, view);
            scoreRail(match.games[0], railTiles[0], paused ? 0.f : dt, art.scoreRails[0], false, "YOU", match.player == 0, gold);
            scoreRail(match.games[1], railTiles[1], paused ? 0.f : dt, art.scoreRails[1], true, roster[opponent].name, match.player == 1,
                      {142, 232, 220, 255});
            title(splitDrill ? "FENWAY 7-10 DRILL" : match.mascot ? (powerRide ? "POWER RIDE" : "STRAIGHT RIDE") : arenas[arena].name,
                  184, 122, 20, cream);
            if (match.mascot && powerRide) {
                arcadeCenter("BONUS +" + std::to_string(match.rideBonus[0]), 304, 76, 18, aqua);
                arcadeCenter("BONUS +" + std::to_string(match.rideBonus[1]), 976, 76, 18, aqua);
            }
            if (art.mascotTokens && turn != Turn::Complete)
                arcadeCenter(std::to_string(tokens.count) + " / 3 TOUCH TANK TOKENS", 948, 142, 21, aqua);
            if (pickupSeconds > 0)
                title(pickupMessage, 172, 685, 25, gold);
            const Power trick = turn == Turn::Ready ? match.queuedTrick[match.player] : match.activeTrick;
            if (trick != Power::None && turn != Turn::Complete)
                arcadeCenter(std::string(powerName(trick)) + " ON THIS SHOT", 965, 177, 23, gold);

            title(turn == Turn::Complete ? "MATCH COMPLETE"
                  : match.player == 0    ? "YOUR TURN"
                                         : "RIVAL'S TURN",
                  184, 148, 19, gold);
            if (strikeSeconds > 0 && strikeSeconds < 1.8f) {
                const float t = strikeSeconds;
                float angle = -74.f * std::exp(-4.8f*t) * std::cos(12*t)
                              - 85.f * PinsetterCycle::ease(t, 1.48f, 1.8f);
                float y = 108.f - 88.f*(1-PinsetterCycle::ease(t, 0, .24f))
                                - 88.f*PinsetterCycle::ease(t, 1.48f, 1.8f);
                BeginScissorMode(322, 99, 215, 132);
                DrawTexturePro(art.strikePennant, {0, 0, float(art.strikePennant.width), float(art.strikePennant.height)},
                               {343, y, 182, 62}, {9, 7}, angle, WHITE);
                EndScissorMode();
            }
            if (turn == Turn::Replay) {
                if (!match.outcome().strike) arcadeCenter(result, 640, 204, 41, gold);
                arcadeCenter("PIN DECK REPLAY", 640, 251, 26, cream);
                button({500, 716, 280, 58}, "SKIP REPLAY / SPACE");
            }
            if (turn == Turn::Reaction) {
                if (strikeSeconds == 0 || strikeSeconds >= 1.8f)
                    title(result, 178, 192, 30, gold);
                if (!rivalLine.empty()) {
                    const float size = std::min(22.f, 345.f * 22 /
                        MeasureTextEx(arcadeFont, rivalLine.c_str(), 22, 1).x);
                    title(rivalLine, 178, 230, size, cream);
                }
            }

            if (celebration > 0 && !strike && turn != Turn::Replay)
                for (int i = 0; i < 65; ++i) {
                    float phase = 2 - celebration;
                    DrawRectangle(int(640 + std::sin(i * 9.f) * phase * 450),
                                  int(280 + std::cos(i * 3.f) * phase * 240 + phase * phase * 90), 7, 5,
                                  i % 2 ? gold : aqua);
                }
            if (turn == Turn::Complete) {
                DrawRectangle(0, 0, W, H, {0, 0, 0, 110});
                panel({292, 187, 696, 458});
                if (stadiumCue == StadiumCue::TropicanaUnlocked || !challengeMessage.empty())
                    sprite(art.sunburst, {319, 211, 642, 287});
                arcadeCenter(result, 640, 231, 39, {170, 255, 64, 255});
                if (splitDrill) {
                    arcadeCenter("FENWAY 7-10 SPLIT", 640, 335, 35, cream);
                    arcadeCenter(reward, 640, 405, 24, gold);
                } else {
                    const int yours = match.total(0), theirs = match.total(1);
                    if (yours == theirs) {
                        portraitCard(portraits, profile.avatar, {435, 280, 184, 200}, "YOU");
                        portraitCard(portraits, opponent, {661, 280, 184, 200}, "RIVAL");
                    } else {
                        const int winner = yours > theirs ? profile.avatar : opponent;
                        portraitCard(portraits, winner, {548, 280, 184, 200}, "TOP BOWLER");
                    }
                    center(reward, 640, 506, std::min(18.f, 600.f * 18 / std::max(1.f, tw(reward, 18))), cream);
                    if (match.mascot && powerRide)
                        arcadeCenter("YOU " + std::to_string(yours) + "   /   RIVAL " + std::to_string(theirs),
                                     640, 532, 23, aqua);
                    else if (!rivalLine.empty())
                        center(rivalLine, 640, 532, 15, cream);
                }
                if (stadiumCue == StadiumCue::TropicanaUnlocked)
                    arcadeCenter("FINAL PARK UNLOCKED: TROPICANA FIELD", 640, 637, 31, gold);
                if (plaqueButton({382, 568, 225, 60}, splitDrill ? "TRY AGAIN" : "REMATCH"))
                    restart();
                if (plaqueButton({673, 568, 225, 60}, splitDrill ? "BACK TO SETUP" : "CLUBHOUSE")) {
                    screen = splitDrill ? Screen::Setup : Screen::Title;
                    splitDrill = paused = charge = false;
                }
            }
            if (!challengeMessage.empty() && (turn == Turn::Complete || turn == Turn::Replay ||
                                              turn == Turn::Reaction))
                arcadeCenter(challengeMessage, 640, 677, 24, gold);
            if (match.mascot && turn == Turn::Ready && human) {
                title(charge ? "PULL STRAIGHT BACK" : "SLIDE THE TUBE TO YOUR SPOT", 180, 194, 25, gold);
                text(charge ? "Release to ride. Backspace cancels."
                            : "Drag back to load the sling, or hold Space.", 180, 225, 16, cream);
                if (charge)
                    arcadeMeter(1045, 500, shot.power, aqua);
            }
            if (!match.mascot && turn == Turn::Ready && human) {
                float x = 240 + (shot.position - geometry::launchMin) /
                                    (geometry::launchMax - geometry::launchMin) * 800;
                const char *labels[] = {"1. SPOT - MOVE THE BALL", "2. CURVE - SHAPE YOUR SHOT",
                                        "3. POWER - TIME YOUR RELEASE"};
                title(labels[int(stadiumSetup.phase)], 180, 194, 27, gold);
                if (splitDrill)
                    title("BANK A CORNER PIN WITH A FIRM INSIDE-EDGE HIT", 180, 287, 21, cream);
                title(stadiumSetup.phase == ShotPhase::Power ? "CLICK / SPACE TO BOWL!"
                                                             : "CLICK / SPACE TO LOCK",
                      180, 227, 22, cream);
                if (stadiumSetup.phase != ShotPhase::Spot)
                    title("BACKSPACE: PREVIOUS STEP", 180, 258, 17, cream);
                if (stadiumSetup.phase == ShotPhase::Spot) {
                    DrawLineEx({x - 28, 535}, {x + 28, 535}, 5, aqua);
                    DrawTriangle({x - 39, 535}, {x - 23, 548}, {x - 23, 522}, aqua);
                    DrawTriangle({x + 39, 535}, {x + 23, 522}, {x + 23, 548}, aqua);
                } else if (stadiumSetup.phase == ShotPhase::Curve) {
                    // A bent flight arrow, not an application slider.
                    Vector2 start{640, 610}, previous = start;
                    for (int i = 1; i <= 24; ++i) {
                        float t = i / 24.f;
                        Vector2 next{640 + shot.curve * 155 * t * t, 610 - 145 * t};
                        DrawLineEx(previous, next, 5, gold);
                        previous = next;
                    }
                    DrawTriangle({previous.x, previous.y - 12}, {previous.x - 10, previous.y + 6},
                                 {previous.x + 10, previous.y + 6}, gold);
                    arcadeCenter(std::abs(shot.curve) < .03f
                                     ? "STRAIGHT"
                                     : std::string(shot.curve < 0 ? "LEFT " : "RIGHT ") +
                                           std::to_string(int(std::abs(shot.curve) * 100)) + "%",
                                 640, 642, 25, cream);
                } else {
                    arcadeMeter(1045, 510, shot.power, gold);
                    arcadeCenter(std::to_string(int(shot.power * 100)) + "%", 1045, 559, 26, gold);
                }
            }
        }
        if (canBrowse && plaqueButton({6, 265, 150, 47}, "GEAR CASE [P]")) {
            equipmentOpen = true;
            charge = stadiumArmed = false;
        }
        if (screen == Screen::Lane && button({8, 724, 144, 64}, "MENU")) {
            paused = true;
            charge = false;
            stadiumArmed = false;
        }
        if (!screenshot.empty() && int(screen) != lastLoggedScreen) {
            std::cout << "UI preview: screen=" << int(screen) << " mascot=" << match.mascot << std::endl;
            lastLoggedScreen = int(screen);
        }
        uiBlocked = false;
        if (equipmentOpen) {
            DrawRectangle(0, 0, W, H, {4, 9, 9, 175});
            sprite(art.gearCase, {155, 45, 970, 705});
            arcadeCenter("DUGOUT EQUIPMENT", 640, 170, 42, cream);
            constexpr const char *effects[] = {"Bigger ball", "Draw toward pins", "Guard the gutters",
                                                "Extra speed", "Shrink rival ball", "Bend rival shot"};
            for (int i = 0; i < powerCount; ++i) {
                float x = 395 + 245.f * (i % 3), y = 236 + 164.f * (i / 3);
                Power power = Power(i + 1);
                bool equipped = match.equipped == power;
                if (equipped)
                    DrawRing({x, y+39}, 45, 49, 0, 360, 60, gold);
                sprite(art.badges[i], {x-42, y, 84, 84}, match.stock[0][i] ? WHITE : LIGHTGRAY);
                arcadeCenter(std::string(powerName(power)) + "  x" + std::to_string(match.stock[0][i]),
                             x, y+83, 24, equipped ? gold : cream);
                center(effects[i], x, y+113, 15, cream);
                if (match.canEquip(power) && ((CheckCollisionPointRec(GetMousePosition(), {x-108,y,216,138}) &&
                    IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) || IsKeyPressed(KEY_ONE+i))) {
                    match.equip(power);
                    audio.play(audio.click);
                }
            }
            if (plaqueButton({523, 583, 234, 54}, "BACK TO LANE"))
                equipmentOpen = false;
        }
        if (paused) {
            DrawRectangle(0, 77, W, 723, {3, 10, 20, 210});
            panel({421, 260, 438, 258}, ink);
            arcadeCenter("TIME OUT!", 640, 294, 42, {170, 255, 64, 255});
            if (plaqueButton({500, 365, 280, 62}, "RESUME") || IsKeyPressed(KEY_ENTER))
                paused = false;
            if (plaqueButton({500, 442, 280, 62}, "LEAVE MATCH")) {
                screen = Screen::Title;
                paused = false;
            }
        }
        EndTextureMode();
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture, {0, 0, float(W), -float(H)},
                       {offset.x, offset.y, W * scale, H * scale}, {0, 0}, 0, WHITE);
        EndDrawing();
        if (smoke && previewTick)
            capture();
        if (smoke && (match.complete() || ticks > int(42 * (geometry::shotTimeout + 3) * 60))) {
            ok = match.complete() && rolls >= 24 && rolls <= 42;
            std::cout << "Native match smoke: mode=" << (match.mascot ? "mascot" : "traditional")
                      << " difficulty=" << difficultyName(match.difficulty)
                      << " rules=" << (match.mascot ? (powerRide ? "power" : "straight") : "stadium")
                      << " bonus=" << match.rideBonus[0] << ',' << match.rideBonus[1]
                      << " complete=" << match.complete() << " rolls=" << rolls
                      << " player=" << (match.complete() ? *match.games[0].scores()[9] : -1)
                      << " rival=" << (match.complete() ? *match.games[1].scores()[9] : -1) << '\n';
            if (!captureAtTick || ticks < captureFrame)
                capture();
            done = true;
        }
        if (!smoke && captureRequested && (ticks == captureFrame || IsKeyPressed(KEY_F12))) {
            capture();
            done = true;
        }
    }
    audio.close();
    UnloadShader(uiBallShader);
    UnloadTexture(uiSlate);
    UnloadTexture(uiFrame);
    UnloadTexture(uiButton);
    UnloadTexture(portraits);
    UnloadTexture(titleArt);
    UnloadFont(arcadeFont);
    UnloadFont(uiFont);
    art.close();
    UnloadShader(antialias);
    UnloadRenderTexture(scene);
    UnloadRenderTexture(target);
    UnloadRenderTexture(replayFeed);
    CloseWindow();
    return smoke && !ok ? 1 : 0;
}
