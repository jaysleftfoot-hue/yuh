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
void scoreCard(const Game &g, float x, Color c, const char *who) {
    panel({x, 8, 610, 77}, {23, 41, 59, 245});
    text(who, x + 12, 16, 17, c);
    auto scores = g.scores();
    for (int i = 0; i < 10; ++i) {
        float left = x + 77 + i * 48.f;
        DrawRectangleRounded({left, 14, 45, 61}, .12f, 4,
                             i == g.frame ? Color{65, 100, 116, 255} : Color{15, 29, 44, 220});
        text(std::to_string(i + 1), left + 3, 17, 9, {145, 177, 188, 255});
        center(marks(g.frames[i], i), left + 23, 25, 15, cream);
        DrawLine(left + 4, 46, left + 41, 46, {72, 103, 115, 255});
        center(scores[i] ? std::to_string(*scores[i]) : "-", left + 23, 51, 18, c);
    }
    int total = 0;
    for (auto score : scores)
        if (score)
            total = *score;
    center(std::to_string(total), x + 36, 43, 24, c);
}
int main(int argc, char **argv) {
    bool smoke = false, startInGame = false, smokeMascot = false, demoShot = false;
    int captureFrame = 90;
    int previewArena = -1;
    std::string screenshot;
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
    auto launch = [&]() {
        if (state == 0 && !paused) {
            match.launch(shot);
            state = 1;
            charge = false;
            audio.play(audio.bowl);
        }
    };
    auto capture = [&]() {
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
            } else if (screen == 2)
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
                        .55f, 9.45f);
                    shot.aim = std::clamp(
                        shot.aim +
                            ((IsKeyDown(KEY_RIGHT) ? 1.f : 0) - (IsKeyDown(KEY_LEFT) ? 1.f : 0)) * dt,
                        -1.f, 1.f);
                    shot.curve = std::clamp(
                        shot.curve + ((IsKeyDown(KEY_E) ? 1.f : 0) - (IsKeyDown(KEY_Q) ? 1.f : 0)) * dt,
                        -1.f, 1.f);
                    bool alleyPointer = !match.mascot && GetMouseY() > 190 && GetMouseY() < 630;
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
                            shot = alleyDrag(shot, GetMouseX() - dragStart.x, dragStart.y - GetMouseY());
                        } else
                            shot.power = .15f + .85f * (.5f - .5f * std::cos(chargeTime * 4));
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
                                         .35f,
                                     -1.f, 1.f);
                if ((human && IsKeyPressed(KEY_SPACE)) || (!human && sim.ball.z > 12) ||
                    (smoke && sim.ball.z > 12))
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
        // The alley reference uses a deliberately compressed arcade perspective.
        laneDepthScale = match.mascot ? (720.f / 41.5f) * 10.f / 21.f : 1.55f;
        camera.fovy = match.mascot ? 43.f : 53.f;
        const auto &view = match.lanes[match.player];
        Vector3 desired{shot.position * .25f - 1.25f, match.mascot ? 4.5f : 2.8f,
                        match.mascot ? -9.0f : -5.8f},
            look{0, match.mascot ? -18.f : 1.2f, laneZ(23)};
        if (screen != 1) {
            desired = match.mascot ? Vector3{-3.2f, 4.5f, -5} : Vector3{-9, 7, -4};
            look = {0, 1, match.mascot ? 13.f : 19.f};
        } else if (state == 1 || state == 2) {
            float z = laneZ(std::min(view.ball.z, 24.f));
            desired = {std::clamp((view.ball.x - 5) * .65f, -3.f, 3.f), match.mascot ? 3.1f : 2.0f,
                       z - (match.mascot ? 5.3f : 5.8f)};
            look = {std::clamp((view.ball.x - 5) * .3f, -1.5f, 1.5f), match.mascot ? 1.5f : .9f, z + 12};
        }
        camera.position =
            Vector3Lerp(camera.position, desired, std::min(1.f, dt * (state == 1 ? 18 : 5)));
        camera.target = Vector3Lerp(camera.target, look, std::min(1.f, dt * (state == 1 ? 18 : 5)));
        BeginTextureMode(scene);
        ClearBackground({45, 92, 129, 255});
        DrawRectangleGradientV(0, 0, W, 550, {17, 39, 75, 255}, {140, 182, 193, 255});
        art.world(view, shot, state == 0 && screen == 1, arenas[arena].team, camera, time);
        EndTextureMode();
        BeginTextureMode(target);
        ClearBackground(BLACK);
        BeginShaderMode(antialias);
        DrawTexturePro(scene.texture, {0, 0, float(W), -float(H)}, {0, 0, float(W), float(H)}, {0, 0}, 0,
                       WHITE);
        EndShaderMode();
        if (screen != 1) {
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
            DrawRectangleGradientH(0, 77, 800, 723, {7, 18, 33, 250}, {7, 18, 33, 0});
            title("BOWL LIKE", 40, 105, 55, cream);
            title("A BIG LEAGUER.", 40, 161, 57, gold);
            if (button({45, 245, 275, 49}, "TRADITIONAL", !match.mascot)) {
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
        } else if (screen == 2) {
            DrawRectangle(0, 78, W, H - 78, {7, 18, 33, 245});
            text("THE BALL LOCKER", 30, 96, 32, gold);
            text("30 clubs. 30 playful remixes. 4 for Tampa Bay.", 500, 110, 19, cream);
            for (int id = 0; id < 64; ++id) {
                int col = id % 8, row = id / 8;
                Rectangle cell{29 + col * 112.f, 153 + row * 70.f, 104, 62};
                bool unlocked = id < profile.balls();
                DrawRectangleRounded(cell, .15f, 5,
                                     id == selectedSkin ? Color{58, 88, 106, 255}
                                                        : Color{21, 37, 56, 255});
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
            scoreCard(match.games[0], 16, gold, "YOU");
            scoreCard(match.games[1], 654, aqua, "RIVAL");
            title(arenas[arena].name, 24, 99, 23, cream);
            title(match.mascot ? "MASCOT RIDE" : "BALLPARK BOWLING", 25, 127, 13, aqua);
            title(state == 3          ? "MATCH COMPLETE"
                  : match.player == 0 ? "YOUR TURN"
                                      : "RIVAL'S TURN",
                  25, 151, 16, gold);
            if (state == 2 || state == 3) {
                DrawRectangle(180, 320, 920, 90, {13, 24, 45, 210});
                center(result, 640, 342, 33, gold);
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
            if ((state != 1 && state != 2) && (match.mascot || state == 3)) {
                panel({24, 691, 1232, 66}, {10, 23, 37, 238});
                slider("POSITION", 39, 702, 160, shot.position, .55f, 9.45f, enabled);
                slider("AIM", 223, 702, 160, shot.aim, -1, 1, enabled);
                slider("POWER", 407, 702, 160, shot.power, .1f, 1, enabled && !charge);
                slider("SPIN", 591, 702, 160, shot.curve, -1, 1, enabled);
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
            if (!match.mascot && state == 0 && human) {
                float x = 240 + (shot.position - .55f) / 8.9f * 800;
                if (!charge) {
                    DrawLineEx({x - 28, 535}, {x + 28, 535}, 5, aqua);
                    DrawTriangle({x - 39, 535}, {x - 23, 548}, {x - 23, 522}, aqua);
                    DrawTriangle({x + 39, 535}, {x + 23, 522}, {x + 23, 548}, aqua);
                    center("MOVE TO POSITION", 640, 705, 18, cream);
                    center("HOLD MOUSE / DRAG UP FOR POWER", 640, 735, 14, aqua);
                } else {
                    panel({x - 21, 438, 42, 149}, ink);
                    float fill = std::clamp((shot.power - .15f) / .85f, 0.f, 1.f) * 129;
                    DrawRectangleRounded({x - 13, 577 - fill, 26, fill}, .2f, 8, aqua);
                    for (int i = 1; i < 8; ++i)
                        DrawLine(x - 13, 577 - i * 16, x + 13, 577 - i * 16, ink);
                    center("DRAG TO AIM / RELEASE TO BOWL", 640, 705, 20, cream);
                    center(std::to_string(int(shot.power * 100)) + "%", x, 596, 17, aqua);
                }
                if (button({24, 737, 150, 38}, "CLUBHOUSE")) {
                    screen = 0;
                    charge = false;
                }
            }
            if (state == 0) {
                const char *names[] = {"1  HEAVY", "2  MAGNET", "3  BUMPERS"};
                for (int i = 0; i < 3; ++i) {
                    std::string label = std::string(names[i]) + "  " + std::to_string(match.stock[0][i]);
                    if (button({28 + i * 140.f, 641, 130, 34}, label, match.equipped == Power(i + 1),
                               enabled && match.stock[0][i] > 0))
                        match.equip(Power(i + 1));
                }
            }
            center(match.mascot
                       ? "A / D  STEER     SPACE  BOOST"
                       : "MOUSE: POSITION / HOLD / RELEASE    |    SPACE: CHARGE    |    ARROWS: AIM",
                   640, 770, 12, {166, 190, 207, 255});
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
        if (smoke && (match.complete() || ticks > 24000)) {
            ok = match.complete() && rolls >= 24 && rolls <= 42;
            std::cout << "Native match smoke: mode=" << (match.mascot ? "mascot" : "traditional")
                      << " complete=" << match.complete() << " rolls=" << rolls
                      << " player=" << (match.complete() ? *match.games[0].scores()[9] : -1)
                      << " rival=" << (match.complete() ? *match.games[1].scores()[9] : -1) << '\n';
            capture();
            done = true;
        }
        if (!smoke && !screenshot.empty() && ticks == captureFrame) {
            capture();
            done = true;
        }
    }
    audio.close();
    UnloadFont(uiFont);
    art.close();
    UnloadShader(antialias);
    UnloadRenderTexture(scene);
    UnloadRenderTexture(target);
    CloseWindow();
    return smoke && !ok ? 1 : 0;
}
