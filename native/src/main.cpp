#include "alley_controls.hpp"
#include "art.hpp"
#include "audio.hpp"
#include "campaign.hpp"
#include <cstdlib>
#include <iostream>
using namespace bowling;
constexpr int W = 1280, H = 800;
enum class Screen { Title, Lane, Locker, Setup, Scores, Help };
enum class Turn { Ready, Rolling, Reaction, Complete };
std::string marks(const std::vector<int> &r, int f) {
    std::string s;
    for (size_t j = 0; j < r.size(); ++j) {
        if (j)
            s += ' ';
        bool fresh =
            j == 0 || (f == 9 && ((j == 1 && r[0] == 10) ||
                                  (j == 2 && (r[0] + r[1] == 10 || (r[0] == 10 && r[1] == 10)))));
        if (r[j] == 10 && fresh)
            s += 'X';
        else if (j && !fresh && r[j - 1] + r[j] == 10)
            s += '/';
        else
            s += r[j] ? std::to_string(r[j]) : "-";
    }
    return s;
}
void scoreCard(const Game &g, float x, const char *who) {
    // One shallow score strip per player, like a classic arcade bowling display.
    panel({x, 0, 610, 67});

    auto scores = g.scores();
    for (int i = 0; i < 10; ++i) {
        float left = x + 9 + i * 51.f;
        if (i == g.frame)
            DrawRectangle(int(left), 3, 49, 43, {88, 123, 132, 125});
        DrawLine(int(left + 50), 6, int(left + 50), 44, {117, 143, 150, 130});
        std::string rolls = marks(g.frames[i], i);
        center(rolls, left + 24, 7, 15, {211, 230, 224, 255});
        if (scores[i])
            center(std::to_string(*scores[i]), left + 24, 31, 23, {150, 242, 77, 255});
    }
    int total = 0;
    for (auto score : scores)
        if (score)
            total = *score;
    center(std::to_string(total), x + 566, 20, 29, Color{150, 242, 77, 255});
    title(who, x + 4, 68, 18, cream);
}
int main(int argc, char **argv) {
    bool smoke = false, startInGame = false, smokeMascot = false, demoShot = false;
    int captureFrame = 90;
    int previewArena = -1;
    RivalDifficulty selectedDifficulty = RivalDifficulty::Pro;
    std::string screenshot, scoreboardScreenshot;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--demo-shot")
            demoShot = true;
        if (a == "--capture-frame" && i + 1 < argc)
            captureFrame = std::clamp(std::atoi(argv[++i]), 1, 3600);
        if (a == "--smoke")
            smoke = true;
        if (a == "--mascot")
            smokeMascot = true;
        if (a == "--difficulty" && i + 1 < argc) {
            std::string difficulty = argv[++i];
            if (difficulty == "rookie")
                selectedDifficulty = RivalDifficulty::Rookie;
            else if (difficulty == "pro")
                selectedDifficulty = RivalDifficulty::Pro;
            else if (difficulty == "all-star")
                selectedDifficulty = RivalDifficulty::AllStar;
            else {
                std::cerr << "Difficulty must be rookie, pro, or all-star\n";
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
    RenderTexture2D target = LoadRenderTexture(W, H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
    Art art;
    art.init();
    art.loadBallSkins();
    art.makeSigns();
    Texture2D titleArt =
        LoadTexture((std::string(GetApplicationDirectory()) + "assets/title-screen.png").c_str());
    if (!titleArt.id)
        throw std::runtime_error("Missing title-screen.png beside executable");
    GenTextureMipmaps(&titleArt);
    SetTextureFilter(titleArt, TEXTURE_FILTER_TRILINEAR);
    Texture2D portraits =
        LoadTexture((std::string(GetApplicationDirectory()) + "assets/baseball-portraits.png").c_str());
    if (!portraits.id)
        throw std::runtime_error("Missing baseball-portraits.png");
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
    Match match;
    match.mascot = smokeMascot;
    Shot shot;
    StadiumSetup stadiumSetup;
    Turn turn = Turn::Ready;
    Screen screen = startInGame || smoke ? Screen::Lane : Screen::Title;
    Screen lockerReturn = Screen::Title;
    int arena = previewArena < 0 ? 0 : previewArena;
    int skin = 0, rolls = 0, ticks = 0, selectedSkin = 0;
    float time = 0, accum = 0, reaction = 0, chargeTime = 0, aiWait = .9f, celebration = 0;
    Vector2 dragStart{};
    bool mouseCharge = false, stadiumArmed = false;
    bool paused = false, charge = false, done = false, ok = false, awarded = false;
    std::string result = "WELCOME TO THE TOUR", reward;
    Camera3D camera{{0, 3.0f, -6.2f}, {0, 1.7f, 23}, {0, 1, 0}, 43, CAMERA_PERSPECTIVE};
    auto resetShot = [&]() {
        shot = Shot{};
        stadiumSetup.reset();
        stadiumArmed = false;
    };
    auto restart = [&]() {
        bool mode = match.mascot;
        match = Match{};
        match.mascot = mode;
        match.stage = arena;
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
            match.launch(shot);
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
        float dt = smoke ? 1.f / 15 : !screenshot.empty() ? 1.f / 60 : std::min(GetFrameTime(), .25f);
        time += dt;
        ++ticks;
        if (demoShot && ticks == 30 && screen == Screen::Lane)
            launch();
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        float scale = std::min(GetScreenWidth() / float(W), GetScreenHeight() / float(H));
        Vector2 offset{(GetScreenWidth() - W * scale) / 2, (GetScreenHeight() - H * scale) / 2};
        SetMouseOffset(-int(offset.x), -int(offset.y));
        SetMouseScale(1 / scale, 1 / scale);
        if (IsKeyPressed(KEY_F11))
            ToggleFullscreen();
        if (IsKeyPressed(KEY_M))
            audio.toggle();
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (screen == Screen::Lane) {
                paused = !paused;
                charge = false;
                stadiumArmed = false;
            } else if (screen != Screen::Title)
                screen = Screen::Title;
            else
                done = true;
        }
        bool human = match.player == 0;
        auto &sim = match.lanes[match.player];
        if (screen == Screen::Lane && !paused) {
            if (turn == Turn::Ready) {
                if (!human) {
                    aiWait -= dt;
                    if (aiWait <= 0) {
                        shot = match.aiShot();
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
                        shot.position = std::clamp(
                            shot.position + (float(IsKeyDown(KEY_D)) - float(IsKeyDown(KEY_A))) * dt * 3,
                            geometry::launchMin, geometry::launchMax);
                        shot.aim = std::clamp(
                            shot.aim + (float(IsKeyDown(KEY_RIGHT)) - float(IsKeyDown(KEY_LEFT))) * dt,
                            -1.f, 1.f);
                        if (IsKeyPressed(KEY_SPACE) ||
                            (alleyPointer && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                            charge = true;
                            mouseCharge = alleyPointer && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                            dragStart = GetMousePosition();
                            chargeTime = 0;
                        }
                        if (charge) {
                            chargeTime += dt;
                            shot = mouseCharge ? mascotPull(shot, GetMouseX() - dragStart.x,
                                                            GetMouseY() - dragStart.y)
                                               : shot;
                            if (!mouseCharge)
                                shot.power = heldPower(chargeTime);
                        }
                        if (charge && ((!mouseCharge && IsKeyReleased(KEY_SPACE)) ||
                                       (mouseCharge && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))))
                            launch();
                    }
                    for (int i = 0; i < 3; ++i)
                        if (IsKeyPressed(KEY_ONE + i))
                            match.equip(Power(i + 1));
                }
            }
            if (turn == Turn::Rolling && match.mascot) {
                if (!human) {
                    match.steering = match.rivalSteering();
                } else if (smoke) {
                    float target = 5 + std::sin(rolls * 2.37f) * (1.6f - arena * .09f);
                    match.steering =
                        std::clamp((target - sim.ball.x) * .65f - sim.ball.vx * .35f, -1.f, 1.f);
                } else {
                    match.steering = float(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                                     float(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A));
                }
                const float boostMark =
                    geometry::foulLine + (geometry::headPin - geometry::foulLine) * .4f;
                if ((human && !smoke && IsKeyPressed(KEY_SPACE)) ||
                    ((!human || smoke) && sim.ball.z > boostMark))
                    match.boost();
            }
            accum += dt;
            while (accum >= 1.f / 120) {
                int impacts = sim.impacts;
                if (turn == Turn::Rolling && match.step(1.f / 120)) {
                    turn = Turn::Reaction;
                    reaction = smoke ? .1f : 1.4f;
                    int n = sim.knocked();
                    result = n == 10  ? "STRIKE! OUT OF THE PARK!"
                             : n == 0 ? "JUST A BIT OUTSIDE!"
                                      : std::to_string(n) + " PINS DOWN!";
                    if (n >= 7) {
                        celebration = 2;
                        audio.play(audio.cheer);
                    }
                }
                if (sim.impacts > impacts)
                    audio.play(audio.hit);
                accum -= 1.f / 120;
            }
            if (turn == Turn::Reaction) {
                reaction -= dt;
                if (reaction <= 0) {
                    match.settle();
                    ++rolls;
                    turn = match.complete() ? Turn::Complete : Turn::Ready;
                    aiWait = smoke ? 0 : .9f;
                    resetShot();
                }
            }
            if (turn == Turn::Complete && !awarded) {
                int yours = *match.games[0].scores()[9], theirs = *match.games[1].scores()[9];
                result = yours > theirs    ? "YOU WIN!"
                         : yours == theirs ? "A TIE! REMATCH?"
                                           : "RIVAL WINS - KEEP SWINGING!";
                int old = profile.balls(), oldA = profile.stadiums();
                int xp = profile.award(yours, theirs, arena);
                reward = "+" + std::to_string(xp) + " XP | " + std::to_string(profile.balls()) +
                         " / 64 BALLS";
                if (profile.balls() > old)
                    reward += " | NEW BALLS!";
                if (profile.stadiums() > oldA)
                    reward += " | NEXT STADIUM OPEN!";
                if (!smoke && screenshot.empty() && previewArena < 0 && !profile.save(save))
                    reward += " | SAVE FAILED";
                awarded = true;
            }
        }
        audio.update();
        celebration = std::max(0.f, celebration - dt);
        if (smoke && ticks % 12 && !match.complete())
            continue;
        art.arena = arena;
        art.skin = skin;
        art.mascot = match.mascot;
        art.slingPower = charge ? shot.power : 0;
        art.updateScoreboard(match);
        const auto cameraStyle = match.mascot ? geometry::mascotCamera : geometry::stadiumCamera;
        camera.fovy = cameraStyle.fov;
        const auto &view = match.lanes[match.player];
        Vector3 desired{geometry::renderX(shot.position) * .25f, cameraStyle.height, cameraStyle.back},
            look{0, cameraStyle.targetHeight, geometry::headPin};
        if (screen != Screen::Lane) {
            desired = match.mascot ? Vector3{-3.2f, 4.5f, -5} : Vector3{-9, 7, -4};
            look = {0, 1, match.mascot ? 13.f : 19.f};
        } else if (turn == Turn::Rolling || turn == Turn::Reaction) {
            float z = std::min(view.ball.z, geometry::headPin + 1);
            desired = {std::clamp(geometry::renderX(view.ball.x) * .65f, -3.f, 3.f),
                       match.mascot ? 3.1f : 2.0f, z - (match.mascot ? 5.3f : 5.8f)};
            look = {std::clamp(geometry::renderX(view.ball.x) * .3f, -1.5f, 1.5f),
                    match.mascot ? 1.5f : .9f, z + 12};
        }
        if (screen == Screen::Lane && turn == Turn::Ready) {
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
        if (screen == Screen::Lane || screen == Screen::Setup)
            art.world(view, shot, turn == Turn::Ready && screen == Screen::Lane, arenas[arena].team,
                      camera, time);
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
        uiBlocked = paused;
        if (screen == Screen::Title) {
            title("WELCOME TO THE BIG LEAGUES!", 565, 16, 20, cream);
            if (button({282, 650, 320, 52}, "SINGLE PLAYER", true) || IsKeyPressed(KEY_ENTER)) {
                match.mascot = false;
                screen = Screen::Setup;
                audio.play(audio.click);
            }
            if (button({660, 650, 320, 52}, "MASCOT ADVENTURE", true)) {
                match.mascot = true;
                screen = Screen::Setup;
                audio.play(audio.click);
            }
            if (button({145, 718, 190, 42}, "BALL LOCKER")) {
                selectedSkin = skin;
                previewAngle = 0;
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
                portraitCard(portraits, 0, {290, 242, 200, 270}, "THE BIG LEAGUER");
                arcadeCenter(std::to_string(profile.wins) + " WINS  /  " +
                                 std::to_string(profile.matches) + " GAMES",
                             770, 267, 32);
                arcadeCenter("BEST GAME  " + std::to_string(profile.best), 770, 325, 34, gold);
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
                arcadeCenter("PULL SIDEWAYS TO AIM. RELEASE TO LAUNCH.", 640, 493, 25);
                arcadeCenter("A / D OR ARROWS: STEER. SPACE: BOOST.", 640, 533, 25);
                center("1 / 2 / 3: power-ups   |   Escape: pause   |   M: sound   |   F11: fullscreen",
                       640, 589, 16, cream);
            }
            if (plaqueButton({516, 650, 248, 64}, "BACK"))
                screen = Screen::Title;
        } else if (screen == Screen::Setup) {
            modal({180, 24, 920, 752}, "BALLPARK SHOWDOWN");
            // Compact versus composition: park preview above two portrait plaques.
            panel({486, 154, 308, 180});
            DrawTexturePro(scene.texture, {0, 0, float(W), -float(H)}, {502, 172, 276, 143}, {0, 0}, 0,
                           WHITE);
            if (button({425, 211, 54, 58}, "<"))
                arena = (arena + 9) % 10;
            if (button({801, 211, 54, 58}, ">"))
                arena = (arena + 1) % 10;
            arcadeCenter(arenas[arena].name, 640, 341, 28);
            portraitCard(portraits, 0, {275, 343, 212, 246}, "PLAYER ONE");
            portraitCard(portraits, 1 + arena % 3, {793, 343, 212, 246}, "THE RIVAL");
            arcadeCenter("VS", 640, 387, 76, {170, 255, 64, 255});
            if (plaqueButton({495, 487, 290, 68}, match.mascot ? "MASCOT RIDE" : "STADIUM BOWLING"))
                match.mascot = !match.mascot;
            if (button({495, 554, 290, 40}, std::string("AI: ") + difficultyName(selectedDifficulty))) {
                selectedDifficulty = static_cast<RivalDifficulty>((int(selectedDifficulty) + 1) % 3);
                audio.play(audio.click);
            }
            bool unlocked = arena < profile.stadiums();
            arcadeCenter(unlocked ? (match.mascot ? "PULL BACK. LET FLY. RIDE THE RAY!"
                                                  : "SPOT. CURVE. POWER. TAKE ON THE RIVAL!")
                                  : "WIN AT " + std::string(arenas[arena - 1].name) + " TO UNLOCK",
                         640, 608, 24, unlocked ? cream : gold);
            if (plaqueButton({395, 650, 226, 65}, unlocked ? "START" : "LOCKED", unlocked) ||
                (IsKeyPressed(KEY_ENTER) && unlocked)) {
                screen = Screen::Lane;
                restart();
                audio.play(audio.cheer);
            }
            if (plaqueButton({659, 650, 226, 65}, "BACK"))
                screen = Screen::Title;
            if (button({275, 594, 212, 35}, "BALL LOCKER")) {
                selectedSkin = skin;
                previewAngle = 0;
                lockerReturn = Screen::Setup;
                screen = Screen::Locker;
            }
            arcadeCenter("PARK " + std::to_string(arena + 1) + " / 10", 900, 598, 20, gold);
        } else if (screen == Screen::Locker) {
            modal({74, 12, 1132, 778}, "THE BALL LOCKER");
            arcadeCenter("30 CLUBS. 30 FUN DESIGNS. 4 RAYS SPECIALS.", 640, 134, 23);
            // Display balls on a single stone cabinet, without individual dashboard cards.
            for (int id = 0; id < 64; ++id) {
                int col = id % 8, row = id / 8;
                Rectangle cell{139 + col * 84.f, 193 + row * 58.f, 78, 55};
                bool unlocked = id < profile.balls();
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
            panel({855, 208, 278, 403});
            arcadeCenter("BALL " + std::to_string(selectedSkin + 1), 994, 237, 31, gold);
            arcadeCenter("ARROW KEYS TO TURN", 994, 271, 15);
            previewAngle = std::fmod(previewAngle + (float(IsKeyDown(KEY_RIGHT)) -
                                                      float(IsKeyDown(KEY_LEFT))) * dt * .32f, 1.f);
            SetShaderValue(uiBallShader, lockerLongitude, &previewAngle, SHADER_UNIFORM_FLOAT);
            ballIcon(art.skins[selectedSkin], 926, 290, 136);
            const float front = 0;
            SetShaderValue(uiBallShader, lockerLongitude, &front, SHADER_UNIFORM_FLOAT);
            std::string name = ballName(selectedSkin);
            float size = std::min(
                24.f, 230.f / std::max(1.f, MeasureTextEx(arcadeFont, name.c_str(), 24, 1).x) * 24);
            arcadeCenter(name, 994, 439, size);
            arcadeCenter(selectedSkin < profile.balls()
                             ? "UNLOCKED"
                             : std::to_string(selectedSkin * 80) + " CAREER XP",
                         994, 484, 22, gold);
            if (plaqueButton({892, 533, 204, 57}, skin == selectedSkin ? "EQUIPPED" : "EQUIP",
                             selectedSkin < profile.balls())) {
                skin = selectedSkin;
                audio.play(audio.click);
            }
            arcadeCenter("EQUIPPED: " + ballName(skin), 475, 676, 22, gold);
            if (plaqueButton({891, 657, 216, 64}, "BACK"))
                screen = lockerReturn;
        } else {
            baseballHud(portraits, 1 + arena % 3, match.lanes[match.player].standingPins());
            scoreCard(match.games[0], 16, "YOU");
            scoreCard(match.games[1], 654, difficultyName(match.difficulty));
            title(match.mascot ? "RAYS ADVENTURE" : arenas[arena].name, 184, 122, 20, cream);

            title(turn == Turn::Complete ? "MATCH COMPLETE"
                  : match.player == 0    ? "YOUR TURN"
                                         : "RIVAL'S TURN",
                  184, 148, 19, gold);
            if (turn == Turn::Reaction) {
                title(result, 260, 245, 46, gold);
            }

            if (celebration > 0)
                for (int i = 0; i < 65; ++i) {
                    float phase = 2 - celebration;
                    DrawRectangle(int(640 + std::sin(i * 9.f) * phase * 450),
                                  int(280 + std::cos(i * 3.f) * phase * 240 + phase * phase * 90), 7, 5,
                                  i % 2 ? gold : aqua);
                }
            bool enabled = turn == Turn::Ready && match.player == 0 && !paused;
            if (turn == Turn::Complete) {
                DrawRectangle(0, 0, W, H, {0, 0, 0, 110});
                panel({292, 187, 696, 458});
                arcadeCenter(result, 640, 231, 39, {170, 255, 64, 255});
                portraitCard(portraits,
                             *match.games[0].scores()[9] >= *match.games[1].scores()[9] ? 0
                                                                                        : 1 + arena % 3,
                             {548, 296, 184, 210}, "TOP BOWLER");
                center(reward, 640, 526, std::min(18.f, 600.f / std::max(1.f, tw(reward, 18)) * 18),
                       cream);
                if (plaqueButton({382, 568, 225, 60}, "REMATCH"))
                    restart();
                if (plaqueButton({673, 568, 225, 60}, "CLUBHOUSE")) {
                    screen = Screen::Title;
                    paused = false;
                    charge = false;
                }
            }
            if (match.mascot && turn == Turn::Ready && human) {
                title(charge ? "LET IT FLY!" : "GRAB THE TUBE", 180, 194, 25, gold);
                text("PULL BACK, AIM, RELEASE", 180, 225, 16, cream);
                if (charge) {
                    arcadeMeter(1045, 500, shot.power, aqua);
                }
            }
            if (!match.mascot && turn == Turn::Ready && human) {
                float x = 240 + (shot.position - geometry::launchMin) /
                                    (geometry::launchMax - geometry::launchMin) * 800;
                const char *labels[] = {"1. SPOT - MOVE THE BALL", "2. CURVE - SHAPE YOUR SHOT",
                                        "3. POWER - TIME YOUR RELEASE"};
                title(labels[int(stadiumSetup.phase)], 180, 194, 27, gold);
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
            {
                for (int i = 0; i < 3; ++i)
                    if (powerButton(i, match.stock[0][i], match.equipped == Power(i + 1),
                                    enabled && match.stock[0][i] > 0))
                        match.equip(Power(i + 1));
            }
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
        if (smoke && (match.complete() || ticks > int(42 * (geometry::shotTimeout + 3) * 60))) {
            ok = match.complete() && rolls >= 24 && rolls <= 42;
            std::cout << "Native match smoke: mode=" << (match.mascot ? "mascot" : "traditional")
                      << " difficulty=" << difficultyName(match.difficulty)
                      << " complete=" << match.complete() << " rolls=" << rolls
                      << " player=" << (match.complete() ? *match.games[0].scores()[9] : -1)
                      << " rival=" << (match.complete() ? *match.games[1].scores()[9] : -1) << '\n';
            capture();
            done = true;
        }
        if (!smoke && !screenshot.empty() && (ticks == captureFrame || IsKeyPressed(KEY_F12))) {
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
    CloseWindow();
    return smoke && !ok ? 1 : 0;
}
