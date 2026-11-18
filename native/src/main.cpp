#include "alley_controls.hpp"
#include "art.hpp"
#include "audio.hpp"
#include "campaign.hpp"
#include <cstdlib>
#include <iostream>
using namespace bowling;
constexpr int W = 1280, H = 800;
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
    return s.empty() ? "." : s;
}
void scoreCard(const Game &g, float x, Color, const char *who) {
    // One shallow score strip per player, like a classic arcade bowling display.
    panel({x, 0, 610, 67}, {66, 84, 91, 255});

    auto scores = g.scores();
    for (int i = 0; i < 10; ++i) {
        float left = x + 9 + i * 51.f;
        if (i == g.frame)
            DrawRectangle(int(left), 3, 49, 43, {88, 123, 132, 125});
        DrawLine(int(left + 50), 6, int(left + 50), 44, {117, 143, 150, 130});
        std::string rolls = marks(g.frames[i], i);
        if (rolls == ".")
            rolls = "";
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
    art.makeSkins();
    art.makeSigns();
    Texture2D titleArt =
        LoadTexture((std::string(GetApplicationDirectory()) + "assets/title-screen.png").c_str());
    if (!titleArt.id)
        throw std::runtime_error("Missing title-screen.png beside executable");
    GenTextureMipmaps(&titleArt);
    SetTextureFilter(titleArt, TEXTURE_FILTER_TRILINEAR);
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
    int state = 0, screen = startInGame || smoke ? 1 : 0, arena = previewArena < 0 ? 0 : previewArena,
        skin = 0, rolls = 0, ticks = 0, selectedSkin = 0;
    float time = 0, accum = 0, reaction = 0, chargeTime = 0, aiWait = .9f, celebration = 0;
    Vector2 dragStart{};
    bool mouseCharge = false;
    bool paused = false, charge = false, done = false, ok = false, awarded = false;
    std::string result = "WELCOME TO THE TOUR", reward;
    Camera3D camera{{0, 3.0f, -6.2f}, {0, 1.7f, 23}, {0, 1, 0}, 43, CAMERA_PERSPECTIVE};
    auto restart = [&]() {
        bool mode = match.mascot;
        match = Match{};
        match.mascot = mode;
        match.stage = arena;
        state = 0;
        charge = false;
        paused = false;
        awarded = false;
        reward.clear();
        shot = Shot{};
        aiWait = .9f;
        result = "MAKE SOME NOISE!";
    };
    restart();
    int lastLoggedScreen = -1;
    auto launch = [&]() {
        if (state == 0 && !paused) {
            match.launch(shot);
            state = 1;
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
        float dt = smoke ? 1.f / 15 : !screenshot.empty() ? 1.f / 60 : std::min(GetFrameTime(), .05f);
        time += dt;
        ++ticks;
        if (demoShot && ticks == 30 && screen == 1)
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
            if (screen == 1) {
                paused = !paused;
                charge = false;
            } else if (screen != 0)
                screen = 0;
            else
                done = true;
        }
        bool human = match.player == 0;
        auto &sim = match.lanes[match.player];
        if (screen == 1 && !paused) {
            if (state == 0) {
                if (!human) {
                    aiWait -= dt;
                    if (aiWait <= 0) {
                        shot = match.aiShot();
                        if (match.games[1].frame % 3 == 1)
                            match.equip(Power(1 + match.games[1].frame % 3));
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
                    shot.position = std::clamp(
                        shot.position +
                            ((IsKeyDown(KEY_D) ? 1.f : 0) - (IsKeyDown(KEY_A) ? 1.f : 0)) * dt * 3,
                        geometry::launchMin, geometry::launchMax);
                    shot.aim = std::clamp(
                        shot.aim +
                            ((IsKeyDown(KEY_RIGHT) ? 1.f : 0) - (IsKeyDown(KEY_LEFT) ? 1.f : 0)) * dt,
                        -1.f, 1.f);
                    shot.curve = std::clamp(
                        shot.curve + ((IsKeyDown(KEY_E) ? 1.f : 0) - (IsKeyDown(KEY_Q) ? 1.f : 0)) * dt,
                        -1.f, 1.f);
                    bool alleyPointer = coursePointer(GetMouseX(), GetMouseY());
                    if (!match.mascot && !charge && alleyPointer &&
                        (GetMouseDelta().x != 0 || GetMouseDelta().y != 0))
                        shot.position = alleyPosition(GetMouseX());
                    if (IsKeyPressed(KEY_SPACE) ||
                        (alleyPointer && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                        charge = true;
                        mouseCharge = alleyPointer && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
                        dragStart = GetMousePosition();
                        chargeTime = 0;
                    }
                    if (charge) {
                        chargeTime += dt;
                        if (mouseCharge) {
                            shot = match.mascot ? mascotPull(shot, GetMouseX() - dragStart.x,
                                                             GetMouseY() - dragStart.y)
                                                : alleyDrag(shot, GetMouseX() - dragStart.x,
                                                            dragStart.y - GetMouseY());
                            if (!match.mascot && std::abs(dragStart.y - GetMouseY()) < 8)
                                shot.power = heldPower(chargeTime);
                        } else
                            shot.power = heldPower(chargeTime);
                    }
                    if (((!mouseCharge && IsKeyReleased(KEY_SPACE)) ||
                         (mouseCharge && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))) &&
                        charge)
                        launch();
                    for (int i = 0; i < 3; ++i)
                        if (IsKeyPressed(KEY_ONE + i))
                            match.equip(Power(i + 1));
                }
            }
            if (state == 1 && match.mascot) {
                match.steering =
                    human && !smoke
                        ? float(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                              float(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
                        : std::clamp((5 + std::sin(rolls * 2.37f) * (1.6f - arena * .09f) - sim.ball.x) *
                                             .65f -
                                         sim.ball.vx * .35f,
                                     -1.f, 1.f);
                if ((human && IsKeyPressed(KEY_SPACE)) ||
                    (!human &&
                     sim.ball.z > geometry::foulLine + (geometry::headPin - geometry::foulLine) * .4f) ||
                    (smoke &&
                     sim.ball.z > geometry::foulLine + (geometry::headPin - geometry::foulLine) * .4f))
                    match.boost();
            }
            accum += dt;
            while (accum >= 1.f / 120) {
                int impacts = sim.impacts;
                if (state == 1 && match.step(1.f / 120)) {
                    state = 2;
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
            if (state == 2) {
                reaction -= dt;
                if (reaction <= 0) {
                    match.settle();
                    ++rolls;
                    state = match.complete() ? 3 : 0;
                    aiWait = smoke ? 0 : .9f;
                    shot = Shot{};
                }
            }
            if (state == 3 && !awarded) {
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
        if (screen != 1) {
            desired = match.mascot ? Vector3{-3.2f, 4.5f, -5} : Vector3{-9, 7, -4};
            look = {0, 1, match.mascot ? 13.f : 19.f};
        } else if (state == 1 || state == 2) {
            float z = std::min(view.ball.z, geometry::headPin + 1);
            desired = {std::clamp(geometry::renderX(view.ball.x) * .65f, -3.f, 3.f),
                       match.mascot ? 3.1f : 2.0f, z - (match.mascot ? 5.3f : 5.8f)};
            look = {std::clamp(geometry::renderX(view.ball.x) * .3f, -1.5f, 1.5f),
                    match.mascot ? 1.5f : .9f, z + 12};
        }
        if (screen == 1 && state == 0) {
            camera.position = desired;
            camera.target = look;
        }
        camera.position =
            Vector3Lerp(camera.position, desired, std::min(1.f, dt * (state == 1 ? 18 : 5)));
        camera.target = Vector3Lerp(camera.target, look, std::min(1.f, dt * (state == 1 ? 18 : 5)));
        BeginTextureMode(scene);
        ClearBackground({45, 92, 129, 255});
        DrawRectangleGradientV(0, 0, W, 550, {17, 39, 75, 255}, {140, 182, 193, 255});
        if (screen == 1 || screen == 3)
            art.world(view, shot, state == 0 && screen == 1, arenas[arena].team, camera, time);
        EndTextureMode();
        BeginTextureMode(target);
        ClearBackground(BLACK);
        BeginShaderMode(antialias);
        DrawTexturePro(scene.texture, {0, 0, float(W), -float(H)}, {0, 0, float(W), float(H)}, {0, 0}, 0,
                       WHITE);
        EndShaderMode();
        if (screen == 0 || screen == 4 || screen == 5) {
            DrawTexturePro(titleArt, {0, 0, float(titleArt.width), float(titleArt.height)},
                           {0, 0, float(W), float(H)}, {0, 0}, 0, WHITE);
        }
        if (screen == 2 || screen == 3) {
            DrawRectangle(0, 0, W, 76, ink);
            DrawRectangle(0, 74, W, 3, gold);
            text("BIG LEAGUE BOWLING", 24, 16, 27, cream);
            text("BALLPARK TOUR", 27, 48, 14, aqua);
            text(arenas[arena].name, 568, 15, 24, gold);
            text(arenas[arena].theme, 570, 46, 14, cream);
            if (button({1110, 18, 142, 36}, audio.muted ? "SOUND OFF" : "SOUND ON"))
                audio.toggle();
        }
        uiBlocked = paused;
        if (screen == 0) {
            title("WELCOME TO THE BIG LEAGUES!", 565, 16, 20, cream);
            if (button({282, 650, 320, 52}, "SINGLE PLAYER", true) || IsKeyPressed(KEY_ENTER)) {
                match.mascot = false;
                screen = 3;
                audio.play(audio.click);
            }
            if (button({660, 650, 320, 52}, "MASCOT ADVENTURE", true)) {
                match.mascot = true;
                screen = 3;
                audio.play(audio.click);
            }
            if (button({145, 718, 190, 42}, "BALL LOCKER")) {
                selectedSkin = skin;
                screen = 2;
            }
            if (button({350, 718, 145, 42}, "SCORES"))
                screen = 4;
            if (button({515, 718, 215, 42}, audio.muted ? "SOUND OFF" : "SOUND ON"))
                audio.toggle();
            if (button({750, 718, 140, 42}, "HELP"))
                screen = 5;
            if (button({920, 718, 140, 42}, "QUIT"))
                done = true;
        } else if (screen == 4 || screen == 5) {
            // A theatrical full-screen interlude, not a dashboard over the title art.
            DrawRectangle(0, 0, W, H, {5, 15, 23, 215});
            if (screen == 4) {
                title("HALL OF FAME", 460, 180, 65, gold);
                center(std::to_string(profile.wins) + " WINS  /  " + std::to_string(profile.matches) +
                           " MATCHES",
                       640, 302, 27, cream);
                center("BEST GAME  " + std::to_string(profile.best), 640, 360, 29, cream);
                center(std::to_string(profile.xp) + " XP   |   " + std::to_string(profile.balls()) +
                           " / 64 BALLS",
                       640, 419, 24, gold);
                center(std::to_string(profile.stadiums()) + " / 10 STADIUMS", 640, 471, 24, aqua);
            } else {
                title("HOW TO BOWL", 455, 160, 65, gold);
                center("STADIUM BOWLING", 640, 277, 28, gold);
                center("Move the mouse to line up. Hold to power up.", 640, 324, 23, cream);
                center("Drag up for power, sideways to aim. Release to bowl.", 640, 361, 23, cream);
                center("MASCOT ADVENTURE", 640, 428, 28, aqua);
                center("Grab the tube. Pull back and sideways, then release.", 640, 475, 23, cream);
                center("A / D or arrows to steer. Space to boost.", 640, 512, 23, cream);
                center("1 / 2 / 3: power-ups     Escape: pause     F11: fullscreen", 640, 570, 19,
                       cream);
            }
            if (button({470, 680, 340, 60}, "BACK TO MAIN MENU", true))
                screen = 0;
        } else if (screen == 3) {
            DrawRectangleGradientH(0, 77, 800, 723, {7, 18, 33, 250}, {7, 18, 33, 0});
            title("BOWL LIKE", 40, 105, 55, cream);
            title("A BIG LEAGUER.", 40, 161, 57, gold);
            if (button({45, 245, 275, 49}, "STADIUM BOWLING", !match.mascot)) {
                match.mascot = false;
                audio.play(audio.click);
            }
            if (button({332, 245, 275, 49}, "MASCOT RIDE", match.mascot)) {
                match.mascot = true;
                audio.play(audio.click);
            }
            text(match.mascot ? "Ride the ray. Steer after launch. Boost into the rack."
                              : "Time the release. Shape your shot. Outsmart your rival.",
                 46, 315, 17, cream);
            panel({43, 355, 565, 158}, {16, 34, 54, 245});
            text("STADIUM " + std::to_string(arena + 1) + " / 10", 65, 369, 16, aqua);
            center(arenas[arena].name, 326, 402, 28, cream);
            center(arenas[arena].theme, 326, 439, 18, gold);
            if (button({57, 405, 43, 38}, "<"))
                arena = (arena + 9) % 10;
            if (button({552, 405, 43, 38}, ">"))
                arena = (arena + 1) % 10;
            center(arena < profile.stadiums()
                       ? "UNLOCKED"
                       : std::string("Win at ") + arenas[arena - 1].name + " to unlock",
                   326, 477, 15, arena < profile.stadiums() ? aqua : gold);
            if (button({45, 538, 563, 60}, "TAKE ON THE RIVAL", true, arena < profile.stadiums()) ||
                (IsKeyPressed(KEY_ENTER) && arena < profile.stadiums())) {
                screen = 1;
                restart();
                audio.play(audio.cheer);
            }
            if (button({45, 624, 275, 48}, "BALL LOCKER")) {
                selectedSkin = skin;
                screen = 2;
            }
            text(std::to_string(profile.balls()) + " / 64 unlocked", 344, 638, 18, gold);
            text("30 TEAM BALLS + 30 FUN DESIGNS + 4 RAYS SPECIALS", 45, 700, 16, cream);
            text("Final destination: TROPICANA FIELD", 45, 738, 20, aqua);
            panel({860, 521, 374, 180}, {12, 29, 47, 238});
            center("YOUR CAREER", 1047, 539, 23, gold);
            center(std::to_string(profile.wins) + " wins  /  " + std::to_string(profile.matches) +
                       " matches",
                   1047, 579, 19, cream);
            center(std::to_string(profile.xp) + " XP  /  Best " + std::to_string(profile.best), 1047,
                   615, 19, aqua);
            center("80 XP unlocks the next ball", 1047, 655, 16, cream);
            if (button({930, 720, 300, 50}, "BACK TO MAIN MENU"))
                screen = 0;
        } else if (screen == 2) {
            DrawRectangle(0, 78, W, H - 78, {7, 18, 33, 245});
            text("THE BALL LOCKER", 30, 96, 32, gold);
            text("30 clubs. 30 playful remixes. 4 for Tampa Bay.", 500, 110, 19, cream);
            for (int id = 0; id < 64; ++id) {
                int col = id % 8, row = id / 8;
                Rectangle cell{29 + col * 112.f, 153 + row * 70.f, 104, 62};
                bool unlocked = id < profile.balls();
                panel(cell, id == selectedSkin ? Color{58, 88, 106, 255} : Color{21, 37, 56, 255});
                DrawTexturePro(art.skins[id], {0, 0, 256, 128}, {cell.x + 25, cell.y + 3, 54, 36},
                               {0, 0}, 0, unlocked ? WHITE : Color{70, 70, 70, 255});
                center(std::to_string(id + 1) + (unlocked ? "" : " LOCK"), cell.x + 52, cell.y + 43, 12,
                       unlocked ? cream : GRAY);
                if (CheckCollisionPointRec(GetMousePosition(), cell) &&
                    IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                    selectedSkin = id;
            }
            panel({950, 167, 300, 457}, ink);
            center("BALL " + std::to_string(selectedSkin + 1), 1100, 189, 23, gold);
            DrawTexturePro(art.skins[selectedSkin], {0, 0, 256, 128}, {977, 243, 246, 123}, {0, 0}, 0,
                           WHITE);
            center(selectedSkin < 30   ? teams[selectedSkin].id
                   : selectedSkin < 60 ? teams[selectedSkin - 30].id
                                       : "RAYS SPECIAL",
                   1100, 393, 25, aqua);
            std::string name = ballName(selectedSkin);
            float fontSize = std::min(19.f, 255.f / std::max(1.f, tw(name, 19)) * 19);
            center(name, 1100, 439, fontSize, cream);
            center(selectedSkin < profile.balls() ? "UNLOCKED"
                                                  : std::to_string(selectedSkin * 80) + " career XP",
                   1100, 479, 18, gold);
            if (button({980, 542, 240, 49}, "EQUIP", true, selectedSkin < profile.balls())) {
                skin = selectedSkin;
                audio.play(audio.click);
            }
            if (button({970, 690, 260, 51}, "BACK TO CLUBHOUSE"))
                screen = 0;
            text("Equipped: " + ballName(skin), 30, 745, 20, aqua);
        } else {
            baseballHud();
            scoreCard(match.games[0], 16, gold, "YOU");
            scoreCard(match.games[1], 654, aqua, "RIVAL");
            title(match.mascot ? "RAYS ADVENTURE" : arenas[arena].name, 184, 122, 20, cream);

            title(state == 3          ? "MATCH COMPLETE"
                  : match.player == 0 ? "YOUR TURN"
                                      : "RIVAL'S TURN",
                  184, 148, 19, gold);
            if (state == 2 || state == 3) {
                title(result, 260, 245, 46, gold);
            }
            if (state == 3)
                center(reward, 640, 429, 18, cream);
            if (celebration > 0)
                for (int i = 0; i < 65; ++i) {
                    float phase = 2 - celebration;
                    DrawRectangle(int(640 + std::sin(i * 9.f) * phase * 450),
                                  int(280 + std::cos(i * 3.f) * phase * 240 + phase * phase * 90), 7, 5,
                                  i % 2 ? gold : aqua);
                }
            bool enabled = state == 0 && match.player == 0 && !paused;
            if ((state != 1 && state != 2) && state == 3) {

                if (button({790, 706, 219, 48}, state == 3 ? "REMATCH" : "BOWL", true,
                           enabled || state == 3)) {
                    if (state == 3)
                        restart();
                    else
                        launch();
                }
                if (button({1030, 706, 209, 48}, "CLUBHOUSE")) {
                    screen = 0;
                    paused = false;
                    charge = false;
                }
            }
            if (match.mascot && state == 0 && human) {
                title(charge ? "LET IT FLY!" : "GRAB THE TUBE", 180, 194, 25, gold);
                text("PULL BACK, AIM, RELEASE", 180, 225, 16, cream);
                if (charge) {
                    arcadeMeter(1045, 500, shot.power, aqua);
                }
            }
            if (!match.mascot && state == 0 && human) {
                float x = 240 + (shot.position - geometry::launchMin) /
                                    (geometry::launchMax - geometry::launchMin) * 800;
                if (!charge) {
                    DrawLineEx({x - 28, 535}, {x + 28, 535}, 5, aqua);
                    DrawTriangle({x - 39, 535}, {x - 23, 548}, {x - 23, 522}, aqua);
                    DrawTriangle({x + 39, 535}, {x + 23, 522}, {x + 23, 548}, aqua);
                    title("MOVE TO LINE UP", 180, 194, 25, gold);
                    text("HOLD TO POWER UP / RELEASE TO BOWL", 180, 225, 16, cream);
                } else {
                    arcadeMeter(x, 510, shot.power, gold);
                    title("RELEASE TO BOWL!", 180, 194, 25, gold);
                    center(std::to_string(int(shot.power * 100)) + "%", x, 596, 17, aqua);
                }
            }
            {
                for (int i = 0; i < 3; ++i)
                    if (powerButton(i, match.stock[0][i], match.equipped == Power(i + 1),
                                    enabled && match.stock[0][i] > 0))
                        match.equip(Power(i + 1));
            }
        }
        if (screen == 1 && button({8, 724, 144, 64}, "MENU")) {
            paused = true;
            charge = false;
        }
        if (!screenshot.empty() && screen != lastLoggedScreen) {
            std::cout << "UI preview: screen=" << screen << " mascot=" << match.mascot << std::endl;
            lastLoggedScreen = screen;
        }
        uiBlocked = false;
        if (paused) {
            DrawRectangle(0, 77, W, 723, {3, 10, 20, 210});
            panel({421, 260, 438, 258}, ink);
            center("TIME OUT!", 640, 294, 42, gold);
            if (button({463, 376, 354, 57}, "RESUME", true) || IsKeyPressed(KEY_ENTER))
                paused = false;
            if (button({463, 450, 354, 43}, "LEAVE MATCH")) {
                screen = 0;
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
