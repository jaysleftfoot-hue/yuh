#pragma once
#include "presentation.hpp"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "ui.hpp"
namespace geo = bowling::geometry;
inline constexpr float sceneryDepthScale = geo::sceneryDepthScale;
struct Art {
    Model cube{}, sphere{}, cylinder{}, pin{}, tube{}, course{}, rayBody{};
    Shader shader{};
    std::array<Texture2D, 64> skins{};
    RenderTexture2D scoreboard{};
    std::array<Texture2D, 10> parkLogos{}, teamLogos{};
    std::string scoreboardKey;
    int arena = 0, skin = 0;
    bowling::MascotPose mascotPose;
    const bowling::PowerPickups *powerPickups = nullptr;
    const bowling::MascotTokens *mascotTokens = nullptr;
    bowling::StadiumCue cue = bowling::StadiumCue::None;
    float cueSeconds = 0, tileSeconds = 0;
    using Tiles = std::array<std::array<std::string, 10>, 2>;
    Tiles visibleTiles{}, oldTiles{};
    bool mascot = false, inSceneryTransform = false;
    Mesh courseMesh() {
        constexpr int rings = 26 + int(geo::sceneryExtension / 2) + 1, sides = 24;
        Mesh m{};
        m.vertexCount = rings * sides * 6;
        m.triangleCount = m.vertexCount / 3;
        m.vertices = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
        m.normals = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
        m.texcoords = (float *)MemAlloc(m.vertexCount * 2 * sizeof(float));
        auto point = [](int r, int j) {
            float a = PI * j / sides, z = -8 + r * 2.f;
            float ripple = .45f * std::sin(j * 1.7f + r * .8f) + .28f * std::sin(r * 2.3f - j);
            return Vector3{std::cos(a) * (8.6f + ripple), .15f + std::sin(a) * (10.5f + ripple * 1.8f),
                           z};
        };
        int n = 0;
        auto tri = [&](Vector3 a, Vector3 b, Vector3 c) {
            Vector3 normal =
                Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a)));
            for (Vector3 v : {a, b, c}) {
                m.vertices[n * 3] = v.x;
                m.vertices[n * 3 + 1] = v.y;
                m.vertices[n * 3 + 2] = v.z;
                m.normals[n * 3] = normal.x;
                m.normals[n * 3 + 1] = normal.y;
                m.normals[n * 3 + 2] = normal.z;
                m.texcoords[n * 2] = v.z * .1f;
                m.texcoords[n * 2 + 1] = v.y * .1f;
                ++n;
            }
        };
        for (int r = 0; r < rings; ++r)
            for (int j = 0; j < sides; ++j) {
                Vector3 a = point(r, j), b = point(r + 1, j), c = point(r + 1, j + 1),
                        d = point(r, j + 1);
                tri(a, b, c);
                tri(a, c, d);
            }
        UploadMesh(&m, false);
        return m;
    }
    Model loadCharacter(const char *name) {
        const auto path = std::string(GetApplicationDirectory()) + "assets/models/" + name + ".glb";
        Model model = LoadModel(path.c_str());
        if (!IsModelValid(model) || model.meshCount != 1)
            throw std::runtime_error("Could not load character mesh: " + path);
        auto &material = model.materials[model.meshMaterial[0]];
        auto &paint = material.maps[MATERIAL_MAP_ALBEDO].texture;
        if (paint.width <= 1)
            throw std::runtime_error("Missing character paint: " + path);
        GenTextureMipmaps(&paint);
        SetTextureFilter(paint, TEXTURE_FILTER_TRILINEAR);
        material.shader = shader;
        return model;
    }
    void init() {
        const char *vs = R"(#version 330
in vec2 vertexTexCoord;out vec2 uv;in vec3 vertexPosition;in vec3 vertexNormal;in vec4 vertexColor;uniform mat4 mvp;uniform mat4 matModel;uniform mat4 matNormal;out vec3 normal;out vec3 world;out vec3 localPos;out vec4 color;
uniform int woodMode;
uniform float sceneTime;
void main(){
localPos=vertexPosition;
vec3 pos=vertexPosition;
if(woodMode==6){
    float wing=pow(clamp((abs(pos.x)-.57)/1.46,0.0,1.0),2.0);
    pos.y+=wing*.18*sin(sceneTime*4.0+pos.z*2.3);
    float tail=clamp((-pos.z-.9)/1.7,0.0,1.0);
    pos.x+=tail*tail*.22*sin(sceneTime*3.0+pos.z*2.0);
}
uv=vertexTexCoord;normal=normalize(mat3(matNormal)*vertexNormal);world=vec3(matModel*vec4(pos,1));color=vertexColor;gl_Position=mvp*vec4(pos,1);})";
        std::string path = std::string(GetApplicationDirectory()) + "assets/lighting.fs";
        char *fs = LoadFileText(path.c_str());
        if (!fs)
            throw std::runtime_error("Missing assets/lighting.fs beside executable");
        shader = LoadShaderFromMemory(vs, fs);
        UnloadFileText(fs);
        cube = LoadModelFromMesh(GenMeshCube(1, 1, 1));
        sphere = LoadModelFromMesh(GenMeshSphere(1, 24, 20));
        cylinder = LoadModelFromMesh(GenMeshCylinder(1, 1, 16));
        pin = loadCharacter("league-pin");
        rayBody = loadCharacter("sunburst-ray");
        tube = loadCharacter("rider-tube");
        course = LoadModelFromMesh(courseMesh());
        for (Model *m : {&cube, &sphere, &cylinder, &course})
            m->materials[0].shader = shader;
    }
    void box(Vector3 p, Vector3 scale, Color c) {
        if (inSceneryTransform && scale.z < 4)
            scale.z /= sceneryDepthScale;
        DrawModelEx(cube, p, {0, 1, 0}, 0, scale, c);
    }
    void orb(Vector3 p, Vector3 scale, Color c) {
        if (inSceneryTransform)
            scale.z /= sceneryDepthScale;
        DrawModelEx(sphere, p, {0, 1, 0}, 0, scale, c);
    }
    void cyl(Vector3 p, float r, float h, Color c) {
        DrawModelEx(cylinder, p, {0, 1, 0}, 0, {r, h, inSceneryTransform ? r / sceneryDepthScale : r},
                    c);
    }
    void beam(Vector3 a, Vector3 b, float r, Color c) {
        DrawCylinderEx(a, b, r, r, 8, c);
    }
    void material(int kind) {
        SetShaderValue(shader, GetShaderLocation(shader, "woodMode"), &kind, SHADER_UNIFORM_INT);
    }
    void makeSigns() {
        scoreboard = LoadRenderTexture(1024, 512);
        for (int i = 0; i < 10; ++i) {
            std::string base = std::string(GetApplicationDirectory()) + "assets/parks/";
            parkLogos[i] = LoadTexture((base + std::to_string(i) + ".png").c_str());
            teamLogos[i] = LoadTexture((base + "team-" + std::to_string(i) + ".png").c_str());
            if (!parkLogos[i].id || !teamLogos[i].id)
                throw std::runtime_error("Missing stadium artwork");
            GenTextureMipmaps(&parkLogos[i]);
            GenTextureMipmaps(&teamLogos[i]);
            SetTextureFilter(parkLogos[i], TEXTURE_FILTER_TRILINEAR);
            SetTextureFilter(teamLogos[i], TEXTURE_FILTER_TRILINEAR);
        }
    }
    void updateScoreboard(const bowling::Match &match, float dt, const Texture2D *replayFeed) {
        int pinsStanding = match.lanes[match.player].standingCount();
        std::string key = std::to_string(arena) + ":" + std::to_string(mascot) + ":" +
                          std::to_string(match.player) + ":" + std::to_string(pinsStanding);
        for (const auto &game : match.games) {
            key += "/" + std::to_string(game.frame);
            for (const auto &frame : game.frames) {
                key += "|";
                for (int roll : frame)
                    key += std::to_string(roll) + ",";
            }
        }
        Tiles nextTiles;
        for (int player = 0; player < 2; ++player) {
            const auto &game = match.games[player];
            const auto scores = game.scores();
            for (int i = 0; i < 10; ++i) {
                const auto &rolls = game.frames[i];
                if (scores[i])
                    nextTiles[player][i] = std::to_string(*scores[i]);
                else if (rolls.empty())
                    nextTiles[player][i] = "-";
                else if (rolls[0] == 10)
                    nextTiles[player][i] = "X";
                else if (rolls.size() >= 2 && rolls[0] + rolls[1] == 10)
                    nextTiles[player][i] = "/";
                else
                    nextTiles[player][i] = std::to_string(rolls[0]);
            }
        }
        const bool flipped = nextTiles != visibleTiles;
        if (flipped) {
            oldTiles = visibleTiles;
            visibleTiles = nextTiles;
            tileSeconds = .42f;
        }
        const bool animating = tileSeconds > 0;
        tileSeconds = std::max(0.f, tileSeconds - dt);
        key += ":" + std::to_string(int(cue)) + ":" + std::to_string(bool(replayFeed));
        if (key == scoreboardKey && !animating && !replayFeed)
            return;
        scoreboardKey = key;
        bool manual = !mascot && (arena == 0 || arena == 1);
        Color paint = manual ? Color{58, 77, 51, 255} : Color{9, 13, 14, 255};
        Color letters = manual ? Color{231, 232, 207, 255} : Color{249, 195, 70, 255};
        BeginTextureMode(scoreboard);
        ClearBackground(paint);
        if (manual) {
            // Hand-operated painted panels, inspired by the supplied Wrigley board.
            for (int row = 0; row < 16; ++row)
                for (int col = 0; col < 32; ++col)
                    DrawRectangleLines(col * 32, row * 32, 32, 32, {42, 58, 38, 150});
            center("NATIONAL", 210, 16, 35, letters);
            center("AMERICAN", 814, 16, 35, letters);
            center("BALLPARK", 512, 24, 20, letters);
            center("FRAME", 512, 99, 22, letters);
            center(std::to_string(std::min(10, match.games[match.player].frame + 1)), 512, 132, 53,
                   letters);
            center("PINS", 512, 216, 19, letters);
            center(std::to_string(pinsStanding), 512, 245, 35, letters);
            const char *names[2][8] = {{"COLORADO", "SAN DIEGO", "LOS ANGELES", "ARIZONA",
                                        "PHILADELPHIA", "WASHINGTON", "PITTSBURGH", "CUBS"},
                                       {"BALTIMORE", "TEXAS", "OAKLAND", "CLEVELAND", "BOSTON",
                                        "TAMPA BAY", "SEATTLE", "NEW YORK"}};
            for (int side = 0; side < 2; ++side) {
                float left = side ? 610 : 20;
                for (int row = 0; row < 8; ++row) {
                    float y = 76 + row * 29.f;
                    text(names[side][row], left + 8, y + 5, 16, letters);
                    DrawLineEx({left, y}, {left + 390, y}, 2, letters);
                }
                DrawLineEx({left, 308}, {left + 390, 308}, 2, letters);
                for (int col = 0; col <= 10; ++col) {
                    float x = left + 180 + col * 21.f;
                    DrawLineEx({x, 61}, {x, 308}, col == 0 ? 3 : 1, Fade(letters, .8f));
                    if (col < 10)
                        center(std::to_string(col + 1), x + 10, 59, 12, letters);
                }
            }
            DrawLineEx({20, 310}, {410, 310}, 4, {158, 42, 35, 255});
            DrawLineEx({610, 310}, {1000, 310}, 4, {158, 42, 35, 255});
            center("BALLPARK BOWLING", 512, 317, 20, letters);
        } else {
            const int park = mascot ? 9 : arena;
            Color accent = GetColor(teams[bowling::arenas[park].team].color);
            DrawRectangleGradientH(0, 0, 1024, 340, {8, 19, 36, 255}, accent);
            // Layered broadcast graphics: angled light ribbons, team insignia and stat panels.
            for (int stripe = 0; stripe < 16; ++stripe) {
                float x = stripe * 90.f - 260;
                DrawTriangle({x, 0}, {x + 230, 340}, {x + 265, 340}, Fade(WHITE, .09f));
                DrawLineEx({x + 20, 0}, {x + 280, 340}, 2, Fade(SKYBLUE, .22f));
            }
            for (int star = 0; star < 25; ++star) {
                int x = (star * 137 + 41) % 1024, y = (star * 67 + 13) % 330;
                DrawLine(x - 3, y, x + 3, y, Fade(WHITE, .35f));
                DrawLine(x, y - 3, x, y + 3, Fade(WHITE, .35f));
            }
            auto fit = [](Texture2D texture, Rectangle rect) {
                float scale = std::min(rect.width / texture.width, rect.height / texture.height);
                DrawTextureEx(texture,
                              {rect.x + (rect.width - texture.width * scale) / 2,
                               rect.y + (rect.height - texture.height * scale) / 2},
                              0, scale, WHITE);
            };
            DrawRectangleGradientV(20, 16, 984, 82, {236, 242, 242, 255}, {173, 193, 204, 255});
            fit(parkLogos[park], {300, 20, 424, 74});
            fit(teamLogos[park], {28, 110, 225, 220});
            for (int side = 0; side < 2; ++side) {
                float x = 280 + side * 360.f;
                DrawRectangleRounded({x, 119, 332, 189}, .04f, 4, {6, 17, 29, 220});
                DrawRectangle(x, 119, 332, 6,
                              side ? Color{91, 201, 221, 255} : Color{255, 206, 87, 255});
                center(side ? "RIVAL" : "YOU", x + 166, 139, 24, WHITE);
                center(std::to_string(bowling::scoreTotal(match.games[side])), x + 166, 172, 85, WHITE);
                center("FRAME " + std::to_string(std::min(10, match.games[side].frame + 1)), x + 166,
                       274, 18, cream);
            }
            letters = WHITE;
        }
        // Dedicated video area. The feed draws only the deck, so it cannot sample this board.
        if (replayFeed) {
            DrawRectangle(132, 13, 760, 323, {7, 18, 25, 255});
            DrawTexturePro(*replayFeed, {0, 0, float(replayFeed->width), -float(replayFeed->height)},
                           {245, 18, 534, 300}, {0, 0}, 0, WHITE);
            arcadeCenter("PIN DECK", 186, 140, 26, gold);
            arcadeCenter("REPLAY", 835, 140, 26, gold);
        }
        if (!replayFeed && cue == bowling::StadiumCue::Turkey) {
            DrawRectangleGradientH(0, 85, 1024, 250, {9, 24, 61, 255}, {34, 101, 173, 255});
            DrawTexturePro(teamLogos[9], {0, 0, float(teamLogos[9].width), float(teamLogos[9].height)},
                           {89, 101, 226, 214}, {0, 0}, 0, WHITE);
            arcadeCenter("TURKEY! RAYS UP!", 640, 165, 83, gold);
        }
        // These frame scores are live game data.
        text("BOWLING", 25, 351, 17, letters);
        for (int frame = 0; frame < 10; ++frame)
            center(std::to_string(frame + 1), 245 + frame * 60, 349, 17, letters);
        center("TOTAL", 945, 349, 16, letters);
        for (int player = 0; player < 2; ++player) {
            const auto &game = match.games[player];
            float y = 377 + player * 45.f;
            text(player ? "RIVAL" : "YOU", 28, y + 6, 24, letters);
            for (int frame = 0; frame < 10; ++frame) {
                float x = 217 + frame * 60;
                DrawRectangleLinesEx({x, y, 56, 37}, 1, Fade(letters, .55f));
                if (frame == game.frame && player == match.player)
                    DrawRectangleLinesEx({x, y, 56, 37}, 3, letters);
                const auto &value = visibleTiles[player][frame];
                const auto &old = oldTiles[player][frame];
                if (manual && tileSeconds > 0 && old != value) {
                    const float phase = 1.f - tileSeconds / .42f;
                    const float faceHeight = std::abs(std::cos(PI * phase));
                    const float height = 35.f * faceHeight;
                    DrawRectangleRec({x + 2, y + 19 - height / 2, 52, height},
                                     ColorBrightness(paint, phase < .5f ? -.18f : .16f));
                    if (faceHeight > .12f)
                        center(phase < .5f ? old : value, x + 28, y + 19 - 11.5f * faceHeight,
                               23 * faceHeight, letters);
                } else
                    center(value, x + 28, y + 8, 23, letters);
            }
            center(std::to_string(bowling::scoreTotal(game)), 945, y + 4, 30, letters);
        }
        DrawRectangle(0, 476, 1024, 36, {5, 8, 9, 255});
        std::string welcome =
            cue == bowling::StadiumCue::TropicanaUnlocked ? "TROPICANA FIELD UNLOCKED"
                   : mascot ? "RAYS UP!" : std::string("WELCOME TO ") + bowling::arenas[arena].name;
        float size = std::min(28.f, 960.f / std::max(1.f, tw(welcome, 28)) * 28);
        center(welcome, 512, 480, size, cream);
        EndTextureMode();
        if (tileSeconds > 0 || replayFeed) {
            // Only the base level changes during playback. Sample it until new mipmaps are ready.
            rlTextureParameters(scoreboard.texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        } else {
            GenTextureMipmaps(&scoreboard.texture);
            SetTextureFilter(scoreboard.texture, TEXTURE_FILTER_TRILINEAR);
        }
    }
    void sign(Texture2D t, Vector3 p, float w, float h, bool image = false) {
        rlSetTexture(t.id);
        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);
        rlNormal3f(0, 0, -1);
        rlTexCoord2f(1, image ? 1 : 0);
        rlVertex3f(p.x - w / 2, p.y - h / 2, p.z);
        rlTexCoord2f(1, image ? 0 : 1);
        rlVertex3f(p.x - w / 2, p.y + h / 2, p.z);
        rlTexCoord2f(0, image ? 0 : 1);
        rlVertex3f(p.x + w / 2, p.y + h / 2, p.z);
        rlTexCoord2f(0, image ? 1 : 0);
        rlVertex3f(p.x + w / 2, p.y - h / 2, p.z);
        rlEnd();
        rlSetTexture(0);
    }
    void loadBallSkins() {
        const std::string folder = std::string(GetApplicationDirectory()) + "assets/balls/";
        for (size_t id = 0; id < skins.size(); ++id) {
            const auto path = folder + (id < 10 ? "0" : "") + std::to_string(id) + ".png";
            skins[id] = LoadTexture(path.c_str());
            if (!skins[id].id)
                throw std::runtime_error("Missing ball artwork: " + path);
            GenTextureMipmaps(&skins[id]);
            SetTextureFilter(skins[id], TEXTURE_FILTER_TRILINEAR);
        }
    }
    void bowlingBall(Vector3 p, float roll, float radius = .51f) {
        rlPushMatrix();
        rlTranslatef(p.x, p.y, p.z);
        rlRotatef(roll, 1, 0, 0);
        Texture2D original = sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture;
        sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skins[skin];
        material(7);
        orb({0, 0, 0}, {radius, radius, radius}, WHITE);
        material(0);
        sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = original;
        rlPopMatrix();
    }
    void drawPin(const bowling::Body &p, float time) {
        rlPushMatrix();
        const float tilt = p.down ? p.fall * 86.f : std::sin(time * 3 + p.id) * 2.f;
        const float tip = std::sin(std::abs(tilt) * DEG2RAD);
        // As the pin tips over, its contact point moves from the foot to the belly.
        const float lift =
            p.down ? std::max(.30f * tip, geo::pinRadius * tip - .9f * std::cos(tilt * DEG2RAD)) : 0.f;
        rlTranslatef(geo::renderX(p.x), .06f + lift, p.z);
        rlRotatef(tilt, p.id % 2 ? 1.f : -1.f, 0, .4f);
        rlScalef(geo::pinRadius / .40f, geo::pinHeight / 2.05f, geo::pinRadius / .40f);
        DrawModel(pin, {0, 0, 0}, 1, WHITE);
        rlPopMatrix();
    }
    void rider(Vector3 p, float time, bool ready, bowling::MascotPose pose, bool gutter) {
        const float bank = pose.bank * 7.f;
        const float wipeout = gutter ? pose.wipeout : 0.f;
        rlPushMatrix();
        rlTranslatef(p.x, p.y + std::sin(time * 3) * .035f - wipeout * .24f, p.z + pose.recoil * .18f);
        rlRotatef(std::sin(time * 3) * 2 - bank * 3.5f + wipeout * (p.x < 0 ? -74.f : 74.f), 0, 0, 1);
        rlRotatef(bank * 2 + wipeout * 55.f, 0, 1, 0);
        DrawModel(tube, {0, 0, 0}, 1, WHITE);
        rlPushMatrix();
        rlTranslatef(0, .12f, 0);
        rlRotatef(ready ? 180.f + std::sin(time * 1.5f) * 5.f : bank * 2, 0, 1, 0);
        rlRotatef(std::sin(time * 4) * 2.5f, 0, 0, 1);
        material(6);
        DrawModel(rayBody, {0, 0, 0}, 1, WHITE);
        material(0);
        rlPopMatrix();
        rlPopMatrix();
    }

    void landmarks(float time) {
        Color brick = {145, 72, 48, 255};
        if (arena == 0)
            for (int i = 0; i < 12; ++i) {
                box({-8.f, 2, 4.f + i * 2.8f}, {1, 4, 2.6f}, brick);
                orb({-7.4f, 3, 4.f + i * 2.8f}, {.5f, 1.7f, 1.6f}, {37, 104, 51, 255});
            }
        if (arena == 1) {
            box({-7, 5, 16}, {1, 10, 35}, {30, 99, 68, 255});
            for (int i = 0; i < 12; ++i)
                box({-6.45f, 5, 1.f + i * 3}, {.05f, 9.5f, .08f}, cream);
        }
        if (arena == 2) {
            box({12, -.02f, 14}, {13, .1f, 42}, {29, 128, 177, 255});
            for (int i = 0; i < 9; ++i)
                box({12, .05f, float(i * 5)}, {9, .02f, .1f}, aqua);
            for (int i = 0; i < 4; ++i)
                box({-9, 1.5f, float(i * 9)}, {3, 3, 3}, brick);
        }
        if (arena == 3)
            for (float z : {4.f, 28.f}) {
                box({-10, 5, z}, {.5f, 10, .5f}, gold);
                box({10, 5, z}, {.5f, 10, .5f}, gold);
                box({0, 10, z}, {21, .45f, .5f}, gold);
                for (int i = -4; i <= 4; ++i)
                    beam({i * 2.f, 10, z}, {i * 2.f, 7, z}, .04f, gold);
            }
        if (arena == 4)
            for (int i = 0; i < 7; ++i) {
                DrawCylinder({-21.f + i * 7, 0, 43}, 0, 7, 14 + (i % 3) * 4, 7, {95, 108, 143, 255});
                DrawCylinder({-21.f + i * 7, 11.f + (i % 3) * 4, 43}, 0, 2.5f, 4, 7, cream);
            }
        if (arena == 5)
            for (int side : {-1, 1})
                for (float z : {7.f, 17.f, 28.f}) {
                    cyl({side * 8.f, 0, z}, 1.5f, .3f, cream);
                    cyl({side * 8.f, .32f, z}, 1.3f, .1f, {25, 129, 186, 255});
                    for (int j = 0; j < 10; ++j) {
                        float a = j * PI / 5;
                        orb({side * 8.f + std::sin(a) * .6f, 1.4f + std::sin(time * 3 + j) * .5f,
                             z + std::cos(a) * .6f},
                            {.08f, 1, .08f}, aqua);
                    }
                }
        if (arena == 6) {
            for (int side : {-1, 1})
                for (float z : {5.f, 15.f, 27.f}) {
                    cyl({side * 9.f, 0, z}, .5f, 5, {42, 126, 66, 255});
                    box({side * 9.f, 3, z}, {2, .5f, .5f}, {42, 126, 66, 255});
                }
            box({11, .1f, 24}, {7, .2f, 12}, {37, 172, 197, 255});
        }
        if (arena == 7) {
            for (float z : {1.f, 12.f, 23.f, 34.f}) {
                box({0, 11, z}, {24, .3f, .4f}, {123, 148, 156, 255});
                for (int side : {-1, 1})
                    box({side * 11.f, 5.5f, z}, {.4f, 11, .4f}, {123, 148, 156, 255});
            }
            box({0, 11.5f, 17}, {26, .25f, 40}, {72, 107, 118, 255});
        }
        if (arena == 8) {
            for (int i = -7; i <= 7; ++i) {
                box({i * 2.f, 5, 34}, {.3f, 4, .5f}, cream);
                box({i * 2.f, 7, 34}, {1.5f, .45f, .6f}, cream);
            }
            box({0, 3, 35}, {31, 1, .6f}, cream);
        }
        if (arena == 9) {
            box({9, 1, 19}, {5, 2, 16}, {31, 104, 149, 255});
            box({9, 2.05f, 19}, {4.5f, .08f, 15.5f}, aqua);
            for (int i = 0; i < 5; ++i)
                orb({9 + std::sin(time + i) * 1.5f, 2.2f, 13 + i * 3.f}, {.65f, .06f, .4f},
                    {24, 75, 108, 255});
        }
    }
    void arch(float z, float width, float height, Color c, float radius) {
        Vector3 last{-width, .35f, z};
        for (int i = 1; i <= 28; ++i) {
            float a = PI * i / 28;
            Vector3 next{-width * std::cos(a), .35f + height * std::sin(a), z};
            // Shader-lit spheres hide the joints between curved rib segments.
            beam(last, next, radius, c);
            last = next;
        }
    }
    void mascotScenery(float time, Color team) {
        const float extra = geo::sceneryExtension;
        Color rock = {45, 119, 165, 255};
        material(3);
        DrawModel(course, {0, 0, 0}, 1, rock);
        box({0, 5.5f, 42.5f + extra}, {20, 11, 1}, rock);
        material(0);
        box({0, 1.3f, 30.8f + extra}, {10, 2.6f, .5f}, {5, 17, 34, 255});
        material(4);
        for (int side : {-1, 1})
            box({side * 6.7f, -.13f, 15 + extra / 2}, {2.45f, .10f, 46 + extra}, {27, 157, 193, 255});
        material(0);
        // Layers of close scenery make the ride feel embedded in a themed course.
        for (int side : {-1, 1})
            for (int i = 0; i < 9 + int(extra / 4.7f) + 1; ++i) {
                float z = -3 + i * 4.7f;
                material(3);
                orb({side * (8.4f + .3f * std::sin(i * 3.f)), 1.2f, z},
                    {1.1f, 1.8f + .8f * std::sin(i * 2.f), 2.4f}, rock);
                material(0);
                if (arena == 0 || arena == 1 || arena == 7) {
                    for (int j = 0; j < 4; ++j)
                        orb({side * (7.8f + j * .08f), 3.f + j * .65f, z + .3f * std::sin(j * 2.f)},
                            {.8f, .7f, 1.1f}, j % 2 ? Color{71, 132, 47, 255} : Color{33, 91, 55, 255});
                }
                cyl({side * 6.35f, .05f, z}, .18f, 1.1f, {22, 53, 86, 255});
                orb({side * 6.35f, 1.2f, z}, {.22f, .25f, .22f}, {135, 246, 255, 255});
            }
        for (float z : {4.f, 16.f + extra / 2, 29.f + extra}) {
            arch(z, 8.1f, 9.3f, {24, 71, 109, 255}, .38f);
            arch(z - .42f, 7.95f, 9.18f, {109, 217, 224, 255}, .09f);
            for (int side : {-1, 1}) {
                box({side * 7.5f, 3.3f, z}, {.7f, 4.5f, .85f}, team);
                box({side * 7.5f, 5.6f, z}, {1.1f, .25f, 1.15f}, gold);
            }
        }
        // A broad themed marquee above the rack, with a readable stadium identity.
        neonMural(30.42f + extra);
        for (int side : {-1, 1}) {
            for (int j = 0; j < 5; ++j)
                orb({side * (5.4f + j * .18f), 2.2f + j * .5f, 30.4f + extra}, {.23f, .23f, .23f}, gold);
            sign(scoreboard.texture, {side * 7.0f, 4.1f, 14.5f + extra / 2}, 3.1f, 1.55f);
        }
        // Tropicana's aquarium theme: suspended rays and bubbles beyond the rails.
        if (mascot)
            for (int side : {-1, 1}) {
                for (int i = 0; i < 3; ++i) {
                    float z = 8 + i * (9.f + extra / 3);
                    const float swimPhase = time * .75f + i * 1.7f + side;
                    const float swimZ = z + std::sin(swimPhase) * 3.4f;
                    const float swimX = side * (7.1f + .25f * std::cos(swimPhase));
                    const float swimY = 4.4f + .35f * std::sin(swimPhase * 2);
                    orb({swimX, swimY, swimZ}, {.65f, .12f, .6f},
                        {30, 70, 121, 255});
                    for (int d : {-1, 1})
                        orb({swimX + d * .65f, swimY + d * .08f * std::cos(swimPhase * 4), swimZ - .1f},
                            {.7f, .06f, .33f}, {31, 82, 133, 255});
                    for (int j = 0; j < 5; ++j) {
                        float y = std::fmod(time * .55f + j * 1.13f + i, 5.f) + 1;
                        orb({side * (6.9f + .2f * std::sin(j * 3.f)), y, z}, {.045f, .065f, .045f},
                            {169, 235, 246, 255});
                    }
                }
            }
    }
    void neonMural(float z) {
        auto tubeLine = [&](Vector3 a, Vector3 b, Color hue) {
            beam(a, b, .095f, {31, 49, 63, 255});
            beam({a.x, a.y, a.z - .12f}, {b.x, b.y, b.z - .12f}, .035f, hue);
        };
        // Original neon bowling illustration, built from continuous tube strokes.
        for (int i = 0; i < 64; ++i) {
            float a = i * 2 * PI / 64, b = (i + 1) * 2 * PI / 64;
            tubeLine({-3.9f + 1.45f * std::cos(a), 5.25f + 1.45f * std::sin(a), z},
                     {-3.9f + 1.45f * std::cos(b), 5.25f + 1.45f * std::sin(b), z}, {69, 232, 194, 255});
        }
        for (Vector2 hole : {Vector2{-4.35f, 5.8f}, Vector2{-3.8f, 6.f}, Vector2{-3.95f, 5.35f}})
            for (int i = 0; i < 16; ++i) {
                float a = i * 2 * PI / 16, b = (i + 1) * 2 * PI / 16;
                tubeLine({hole.x + .12f * std::cos(a), hole.y + .12f * std::sin(a), z},
                         {hole.x + .12f * std::cos(b), hole.y + .12f * std::sin(b), z},
                         {69, 232, 194, 255});
            }
        const Vector2 shape[] = {
            {-.48f, 0},     {-.68f, .36f},  {-.72f, .82f}, {-.61f, 1.25f}, {-.29f, 1.9f}, {-.22f, 2.36f},
            {-.39f, 2.65f}, {-.36f, 3.02f}, {0, 3.19f},    {.36f, 3.02f},  {.39f, 2.65f}, {.22f, 2.36f},
            {.29f, 1.9f},   {.61f, 1.25f},  {.72f, .82f},  {.68f, .36f},   {.48f, 0},     {-.48f, 0}};
        for (int pin = 0; pin < 3; ++pin) {
            float angle = (pin - 1) * .25f, ox = .2f + pin * 1.7f;
            auto point = [&](Vector2 p) {
                return Vector3{ox + p.x * std::cos(angle) - p.y * std::sin(angle),
                               3.7f + p.x * std::sin(angle) + p.y * std::cos(angle), z};
            };
            for (int i = 0; i < 17; ++i)
                tubeLine(point(shape[i]), point(shape[i + 1]), {233, 111, 220, 255});
            tubeLine(point({-.24f, 2.2f}), point({.24f, 2.2f}), {255, 202, 108, 255});
        }
        for (int i = 0; i < 28; ++i) {
            float x = -6.3f + i * .45f;
            tubeLine({x, 3.45f + .1f * std::sin(x), z}, {x + .45f, 3.45f + .1f * std::sin(x + .45f), z},
                     {255, 184, 99, 255});
        }
    }
    void loungeDetails(Color team) {
        // Close columns, soffits and pendant lights, in the warm arcade-bowling style.
        for (int side : {-1, 1})
            for (float z : {-1.f, 10.f, 21.f}) {
                material(2);
                box({side * 13.f, 3.8f, z}, {1.15f, 7.6f, 1.15f}, {139, 64, 49, 255});
                material(0);
                box({side * 13.f, .5f, z}, {1.5f, 1, 1.5f}, {37, 57, 66, 255});
                box({side * 13.f, 7.35f, z}, {1.65f, .6f, 1.65f}, gold);
                // Long tables sit between outer lanes, clear of the playable surface.
                box({side * 13.f, 1.0f, z + 3}, {1.05f, .25f, 3.3f}, {148, 73, 39, 255});
                for (float dz : {-1.f, 1.f}) {
                    cyl({side * 13.f, 0, z + 3 + dz}, .09f, .9f, ink);
                    cyl({side * 13.f, 1.16f, z + 3 + dz}, .15f, .45f, red);
                    cyl({side * 13.f, 1.62f, z + 3 + dz}, .17f, .04f, cream);
                    beam({side * 13.f, 1.63f, z + 3 + dz}, {side * 13.f + .05f, 1.9f, z + 3 + dz}, .015f,
                         cream);
                }
            }
        for (int x = -3; x <= 3; ++x)
            for (int row = 0; row < 5; ++row) {
                float z = -5 + row * 9.f;
                box({x * 8.f, 8.95f, z}, {7.8f, .28f, 8.8f},
                    (x + row) % 2 ? Color{73, 93, 88, 255} : Color{117, 74, 66, 255});
                box({x * 8.f, 8.78f, z}, {6.6f, .06f, 7.6f},
                    (x + row) % 2 ? Color{91, 113, 103, 255} : Color{135, 91, 73, 255});
            }
        for (float z : {1.f, 12.f, 23.f}) {
            box({0, 8.5f, z}, {29, .45f, 1.2f}, {84, 39, 49, 255});
            box({0, 8.24f, z - .48f}, {28, .06f, .08f}, {255, 179, 100, 255});
            for (float x : {-8.f, 0.f, 8.f}) {
                beam({x, 8.3f, z}, {x, 6.9f, z}, .035f, ink);
                DrawCylinder({x, 6.6f, z}, .38f, .85f, .5f, 24, team);
                cyl({x, 6.34f, z}, .81f, .035f, {255, 223, 155, 255});
            }
        }
        // Rounded neon line work on the pinsetter wall, visible under the sign.
        for (int side : {-1, 1}) {
            float x = side * 10.f;
            for (int i = 0; i < 32; ++i) {
                float a = i * 2 * PI / 32, b = (i + 1) * 2 * PI / 32;
                beam({x + std::cos(a) * 2.2f, 4.9f + std::sin(a) * 1.2f, 28.4f},
                     {x + std::cos(b) * 2.2f, 4.9f + std::sin(b) * 1.2f, 28.4f}, .055f,
                     side < 0 ? Color{250, 107, 119, 255} : Color{76, 236, 201, 255});
            }
        }
    }
    void fieldPath(Vector3 a, Vector3 b, float width, Color tint) {
        Vector3 d = Vector3Subtract(b, a);
        rlPushMatrix();
        rlTranslatef((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);
        rlRotatef(std::atan2(d.x, d.z) * RAD2DEG, 0, 1, 0);
        box({0, 0, 0}, {width, .035f, Vector3Length(d)}, tint);
        rlPopMatrix();
    }
    void logoSign(int park, Vector3 position, float width, float height) {
        auto texture = parkLogos[park];
        float aspect = float(texture.width) / texture.height;
        float w = std::min(width, height * aspect), h = w / aspect;
        // sign() expects render textures (inverted Y); loaded PNGs use opposite UVs.
        sign(texture, position, w, h, true);
    }
    void stadiumScenery(float time, Color team) {
        // The playable lane sits on the diamond, with a full ballpark around it.
        const float extra = geo::sceneryExtension;
        material(0);
        for (int stripe = 0; stripe < 18 + int(extra / 4) + 2; ++stripe)
            box({0, -.24f, -12 + stripe * 4.f}, {118, .12f, 4.01f},
                stripe % 2 ? Color{47, 111, 57, 255} : Color{58, 129, 65, 255});
        for (int side : {-1, 1}) {
            fieldPath({0, -.13f, -8}, {side * 21.f, -.13f, 10}, 2.8f, {166, 112, 67, 255});
            fieldPath({side * 21.f, -.13f, 10}, {0, -.13f, 30}, 2.8f, {166, 112, 67, 255});
            fieldPath({0, .01f, -8}, {side * 48.f, .01f, 34}, .10f, cream);
            box({side * 21.f, .03f, 10}, {.8f, .08f, .8f}, cream);
            // Field-level dugouts with padded rails.
            box({side * 16.f, .6f, 0}, {5, 1.2f, 7}, {16, 51, 54, 255});
            box({side * 16.f, 2.8f, 0}, {5.5f, .25f, 7.5f}, team);
            beam({side * 13.5f, 1.3f, -3.5f}, {side * 13.5f, 1.3f, 3.5f}, .10f, team);
        }
        box({0, .03f, -8}, {.8f, .08f, .8f}, cream);
        box({0, .03f, 30}, {.8f, .08f, .8f}, cream);
        // Low pin return keeps the stadium visible above the deck.
        box({0, .6f, 31 + extra}, {11, 1.2f, 1.2f}, {11, 28, 43, 255});
        box({0, 1.25f, 31 + extra}, {11.3f, .12f, 1.3f}, team);
        // Continue the seating down both sidelines as lane length increases.
        for (int side : {-1, 1})
            for (float z = -8; z < extra + 28; z += 7.8f) {
                for (int row = 0; row < 8; ++row) {
                    float x = side * (43 + row * .7f), y = 4 + row * .8f;
                    box({x, y / 2, z}, {1, y, 8}, {91, 103, 111, 255});
                    for (int seat = 0; seat < 8; ++seat) {
                        DrawCubeV({x, y, z - 3.4f + seat * .9f}, {.5f, .5f, .3f}, team);
                        if (seat % 3) {
                            DrawCubeV({x, y + .45f, z - 3.4f + seat * .9f}, {.35f, .5f, .25f},
                                      {172, 180, 177, 255});
                            DrawSphereEx({x, y + .83f, z - 3.4f + seat * .9f}, .15f, 4, 6,
                                         {178, 138, 108, 255});
                        }
                    }
                }
            }
        for (int section = 0; section < 19; ++section) {
            float a = (-85 + section * 10) * DEG2RAD;
            float x = std::sin(a) * 43, z = 13 + extra + std::cos(a) * 30;
            rlPushMatrix();
            rlTranslatef(x, 0, z);
            rlRotatef(a * RAD2DEG, 0, 1, 0);
            box({0, 2, 0}, {8.2f, 4, 1.0f}, arena == 1 ? Color{29, 95, 66, 255} : team);
            box({0, 4.15f, 0}, {8.3f, .18f, 1.3f}, {226, 199, 139, 255});
            for (int tier = 0; tier < 2; ++tier) {
                for (int row = 0; row < 5; ++row) {
                    float y = 4.6f + tier * 4.4f + row * .61f, depth = 1 + row * .8f + tier * 4;
                    box({0, y * .5f, depth}, {8.2f, y, 1.0f}, {99, 112, 119, 255});
                    for (int seat = 0; seat < 9; ++seat) {
                        float sx = -3.3f + seat * .82f;
                        DrawCubeV({sx, y, depth}, {.56f, .55f, .175f}, team);
                        if ((seat + row + section) % 3) {
                            DrawCubeV({sx, y + .45f, depth}, {.38f, .52f, .16f},
                                      (seat + row) % 2 ? Color{145, 172, 184, 255} : team);
                            DrawSphereEx({sx, y + .8f, depth}, .15f, 4, 6, {178, 138, 108, 255});
                        }
                    }
                }
                box({0, 8.1f + tier * 4.4f, 5 + tier * 4.f}, {8.3f, .3f, 1.4f}, cream);
            }
            rlPopMatrix();
        }
        for (int side : {-1, 1})
            for (float z : {7.f, 37.f + extra}) {
                cyl({side * 36.f, 0, z}, .18f, 22, {113, 133, 144, 255});
                box({side * 36.f, 22, z}, {5.4f, 2.5f, .4f}, {32, 49, 60, 255});
                for (int row = 0; row < 2; ++row)
                    for (int col = 0; col < 5; ++col)
                        orb({side * 36.f - 2 + col, 21.5f + row, z - .3f}, {.28f, .28f, .08f},
                            {255, 235, 185, 255});
            }
        // Stadium identity belongs on the outfield infrastructure, not over the pins.
        Color boardPaint = (arena == 0 || arena == 1) ? Color{58, 77, 51, 255} : Color{19, 23, 25, 255};
        float boardX = arena == 9 ? 0.f : -15.f, boardW = arena == 9 ? 39.f : (arena == 0 ? 19.f : 23.f);
        float boardY = arena == 9 ? 16.f : (arena == 0 ? 12.f : 14.f),
              boardH = arena == 9 ? 12.f : (arena == 0 ? 9.3f : 11.5f);
        box({boardX, boardY, 42 + extra}, {boardW + .7f, boardH + .7f, .8f}, boardPaint);
        sign(scoreboard.texture, {boardX, boardY, 41.5f + extra}, boardW, boardH);
        if (arena != 0) {
            beam({boardX - boardW / 2, boardY + boardH / 2 + 1, 42 + extra},
                 {boardX + boardW / 2, boardY + boardH / 2 + 1, 42 + extra}, .12f, {110, 126, 137, 255});
            logoSign(arena, {boardX, boardY + boardH / 2 + 3.4f, 41.55f + extra}, boardW - 1, 6.2f);
        }
        // Exposed steel legs and diagonal trusses keep the board a stadium structure.
        for (int side : {-1, 1})
            for (int level = 0; level < 5; ++level) {
                float x = boardX + side * (boardW / 2 + .8f), y = level * 4.f;
                beam({x, y, 42 + extra}, {x, y + 4, 42 + extra}, .12f, {85, 97, 103, 255});
                beam({x - side * 1.6f, y, 42 + extra}, {x - side * 1.6f, y + 4, 42 + extra}, .12f,
                     {85, 97, 103, 255});
                beam({x, y, 42 + extra}, {x - side * 1.6f, y + 4, 42 + extra}, .08f,
                     {131, 144, 151, 255});
            }
        for (float x : {-22.f, -8.f})
            cyl({x, 0, 42 + extra}, .22f, 8, {66, 77, 72, 255});
        for (int lamp = -3; lamp <= 3; ++lamp) {
            float x = -15 + lamp * 2.8f;
            beam({x, 16.8f, 42 + extra}, {x, 17.5f, 41.5f + extra}, .07f, {103, 116, 110, 255});
            box({x, 17.5f, 41.4f + extra}, {.7f, .4f, .35f}, {31, 40, 36, 255});
            box({x, 17.45f, 41.15f + extra}, {.52f, .24f, .06f}, cream);
        }
        if (arena == 0) {
            // Wrigley's distinctive clock above its manual green scoreboard.
            beam({-15, 16.8f, 42 + extra}, {-15, 19, 42 + extra}, .20f, boardPaint);
            orb({-15, 19, 41.9f + extra}, {2, 2, .2f}, cream);
            orb({-15, 19, 41.65f + extra}, {1.88f, 1.88f, .15f}, boardPaint);
            for (int tick = 0; tick < 12; ++tick) {
                float a = tick * 2 * PI / 12;
                orb({-15 + std::sin(a) * 1.65f, 19 + std::cos(a) * 1.65f, 41.45f + extra},
                    {.075f, .075f, .03f}, cream);
            }
            beam({-15, 19, 41.4f + extra}, {-16.1f, 20, 41.4f + extra}, .085f, cream);
            beam({-15, 19, 41.4f + extra}, {-15.1f, 20.5f, 41.4f + extra}, .06f, cream);
            for (float x : {-23.f, -7.f}) {
                beam({x, 16.8f, 42 + extra}, {x, 21, 42 + extra}, .05f, cream);
                DrawTriangle3D({x, 21, 42 + extra}, {x, 19.8f, 42 + extra},
                               {x + 1.9f, 20.4f, 42 + extra + .1f * std::sin(time)}, team);
            }
        }
        for (int side : {-1, 1}) {
            cyl({side * 39.f, 0, 32 + extra}, .10f, 16, gold);
            box({side * 39.f, 14.3f, 32 + extra}, {1.8f, 2.4f, .08f}, gold);
        }
        // Individual landmark silhouettes visible beyond the diamond.
        rlPushMatrix();
        rlTranslatef(0, 0, 18 + extra);
        rlScalef(2.1f, 1.0f, .75f);
        landmarks(time);
        rlPopMatrix();
        if (arena == 9) {
            for (float z : {-2.f, 14.f + extra / 3, 30.f + extra * .66f, 46.f + extra})
                arch(z, 49, 31, {155, 184, 200, 255}, .15f);
            box({0, 32, 20 + extra / 2}, {115, .3f, 90 + extra}, {167, 174, 175, 255});
            // Tropicana's roof catwalk, orange landmark and flanking display pylons.
            for (float y : {27.f, 28.f})
                beam({-43, y, 40 + extra}, {43, y, 40 + extra}, .12f, {48, 59, 67, 255});
            for (int x = -42; x <= 42; x += 3)
                beam({float(x), 27, 40 + extra}, {float(x), 28, 40 + extra}, .07f, cream);
            orb({28, 17, 41 + extra}, {3.2f, 3.2f, 1.5f}, {255, 170, 16, 255});
            beam({29, 19, 41 + extra}, {31, 23, 41 + extra}, .17f, {245, 233, 198, 255});
            for (int side : {-1, 1}) {
                box({side * 25.f, 13, 42 + extra}, {5, 16, .7f}, {181, 197, 202, 255});
                logoSign(9, {side * 25.f, 14, 41.5f + extra}, 4.6f, 8.f);
            }
        }
    }
    void world(const bowling::Simulation &sim, const bowling::Shot &shot, bool aim, int selected,
               Camera3D cam, float time, bool deckOnly = false) {
        Color team = mascot ? Color{16, 52, 94, 255} : GetColor(teams[selected].color);
        SetShaderValue(shader, GetShaderLocation(shader, "eye"), &cam.position, SHADER_UNIFORM_VEC3);
        Vector3 shadow[10];
        for (int i = 0; i < 10; ++i)
            shadow[i] = {1000, 0, 1000};
        for (size_t i = 0; i < sim.pins.size(); ++i)
            shadow[i] = {geo::renderX(sim.pins[i].x), 0, sim.pins[i].z};
        SetShaderValueV(shader, GetShaderLocation(shader, "pinShadows"), shadow, SHADER_UNIFORM_VEC3,
                        10);
        Vector3 ballPos{geo::renderX(sim.hasBall ? sim.ball.x : shot.position), 0,
                        sim.hasBall ? sim.ball.z : geo::foulLine};
        Vector3 shadowBall{ballPos.x, 0, ballPos.z};
        SetShaderValue(shader, GetShaderLocation(shader, "ballShadow"), &shadowBall,
                       SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "sceneTime"), &time, SHADER_UNIFORM_FLOAT);
        const float glow = cue == bowling::StadiumCue::Turkey
                               ? std::min(1.f, cueSeconds) * (.65f + .35f * std::sin(time * 6))
                               : 0.f;
        const Vector2 boardLight{glow, geo::headPin};
        SetShaderValue(shader, GetShaderLocation(shader, "boardLight"), &boardLight, SHADER_UNIFORM_VEC2);
        BeginMode3D(cam);
        if (!deckOnly) {
            rlPushMatrix();
            rlTranslatef(0, 0, 2.f - 2.f * sceneryDepthScale);
            rlScalef(1, 1, sceneryDepthScale);
            inSceneryTransform = true;
            material(0);
            box({0, -.7f, 15}, {95, 1, 95}, {17, 39, 43, 255});
            if (!mascot)
                stadiumScenery(time, team);
            else
                mascotScenery(time, {16, 52, 94, 255});
            rlPopMatrix();
            inSceneryTransform = false;
        }
        material(mascot ? 5 : 1);
        box({0, -.06f, geo::surfaceCenter}, {geo::width, .2f, geo::surfaceLength},
            mascot ? Color{158, 213, 224, 255} : Color{222, 171, 103, 255});
        material(0);
        for (float x : {-geo::gutterCenter, geo::gutterCenter}) {
            box({x, -.08f, geo::surfaceCenter}, {geo::gutterWidth, .15f, geo::surfaceLength},
                {16, 37, 55, 255});
            for (float d : {-.30f, .30f})
                beam({x + d, .10f, geo::surfaceStart}, {x + d, .10f, geo::surfaceEnd}, .07f,
                     {107, 156, 176, 255});
            box({x + (x < 0 ? -.5f : .5f), .12f, geo::surfaceCenter}, {.25f, .42f, geo::surfaceLength},
                team);
            box({x + (x < 0 ? -.5f : .5f), .36f, geo::surfaceCenter}, {.15f, .025f, geo::surfaceLength},
                aqua);
        }
        box({0, .047f, geo::foulLine}, {geo::width, .013f, .13f}, red);
        for (int i = -3; i <= 3; ++i) {
            float z = geo::arrowZ + std::abs(i) * .64f;
            DrawTriangle3D({i * .95f - .12f, .051f, z}, {i * .95f, .051f, z + .32f},
                           {i * .95f + .12f, .051f, z}, ink);
        }
        for (int i = -2; i <= 2; ++i)
            DrawSphere({i * 1.1f, .05f, geo::dotsZ}, .045f, ink);
        if (!deckOnly && (aim || sim.active)) {
            if (powerPickups)
                for (const auto &item : powerPickups->items) {
                    if (item.claimed)
                        continue;
                    const float x = geo::renderX(item.position.x);
                    const float y = 1.04f + .13f * std::sin(time * 3.2f + item.position.z);
                    orb({x, y, item.position.z}, {.38f, .38f, .38f},
                        powerColors[int(item.power) - 1]);
                }
            if (mascotTokens)
                for (size_t i = 0; i < mascotTokens->layout.size(); ++i) {
                    if (mascotTokens->collected[i])
                        continue;
                    const auto token = mascotTokens->layout[i];
                    orb({geo::renderX(token.x), .82f + .14f * std::sin(time * 4 + i), token.z},
                        {.55f, .46f, .30f}, gold);
                }
        }
        for (const auto &p : sim.pins)
            drawPin(p, time);
        if (ballPos.z < geo::surfaceEnd) {
            if (mascot)
                rider({ballPos.x + (aim ? mascotPose.sideways * 1.6f : 0), .57f,
                       ballPos.z - (aim ? mascotPose.pull * 2.7f : 0)},
                      time, aim, mascotPose, sim.gutter);
            else
                bowlingBall({ballPos.x, sim.gutter ? .28f : (sim.hasBall ? sim.ball.r : geo::ballRadius),
                             ballPos.z},
                            sim.hasBall ? (ballPos.z - geo::foulLine) * RAD2DEG / sim.ball.r : 0.f,
                            sim.hasBall ? sim.ball.r : geo::ballRadius);
        }
        if (aim && mascot) {
            float pull = mascotPose.pull * 2.7f;
            // Sling attaches to the front of the tube, below the ray's fins.
            Vector3 center{ballPos.x + mascotPose.sideways * 1.6f, .40f, ballPos.z - pull - 1.f};
            for (int side : {-1, 1}) {
                cyl({side * 4.8f, .04f, 2.4f}, .24f, 1.3f, {24, 69, 110, 255});
                orb({side * 4.8f, 1.4f, 2.4f}, {.31f, .34f, .31f}, gold);
                beam({side * 4.8f, 1.15f, 2.4f}, center, .095f - .03f * mascotPose.pull,
                     {220, 44, 60, 255});
            }
        }
        if (aim)
            for (int i = 0; i < 20; ++i) {
                float t = i * .07f,
                      x = geo::renderX(shot.position +
                                       std::sin(shot.aim * .16f) * geo::launchSpeed(shot.power) * t +
                                       .5f * shot.curve * .85f * t * t);
                if (std::abs(x) < 4.7f)
                    orb({x, .07f, geo::foulLine + t * geo::launchSpeed(shot.power)},
                        {.045f, .015f, .10f}, gold);
            }
        EndMode3D();
    }
    void close() {
        for (Model *character : {&pin, &rayBody, &tube})
            UnloadTexture(
                character->materials[character->meshMaterial[0]].maps[MATERIAL_MAP_ALBEDO].texture);
        for (Model m : {cube, sphere, cylinder, pin, tube, course, rayBody})
            UnloadModel(m);
        for (auto t : skins)
            UnloadTexture(t);
        for (auto t : parkLogos)
            UnloadTexture(t);
        for (auto t : teamLogos)
            UnloadTexture(t);
        UnloadRenderTexture(scoreboard);
        UnloadShader(shader);
    }
};
