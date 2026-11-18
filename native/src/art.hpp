#pragma once
#include "campaign.hpp"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "ui.hpp"
namespace geo = bowling::geometry;
inline constexpr float sceneryDepthScale = geo::sceneryDepthScale;
struct Art {
    Model cube{}, sphere{}, cylinder{}, pin{}, wing{}, tube{}, course{};
    Shader shader{};
    std::array<Texture2D, 64> skins{};
    RenderTexture2D scoreboard{};
    std::array<Texture2D, 10> parkLogos{}, teamLogos{};
    std::string scoreboardKey;
    int arena = 0, skin = 0;
    float slingPower = 0;
    bool mascot = false, inSceneryTransform = false;
    // Continuous shaded tube and an irregular, inward-facing stadium grotto.
    Mesh tubeMesh() {
        Mesh m = GenMeshTorus(.30f, 1.0f, 24, 64);
        return m;
    }
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
    Mesh wingMesh() {
        constexpr int nx = 16, nz = 12;
        Mesh m{};
        m.vertexCount = nx * nz * 12;
        m.triangleCount = m.vertexCount / 3;
        m.vertices = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
        m.normals = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
        m.texcoords = (float *)MemAlloc(m.vertexCount * 2 * sizeof(float));
        int n = 0;
        auto point = [](float u, float v, float side) {
            float width = (1 - u) * .8f + .03f;
            return Vector3{.3f + u * 1.15f,
                           .16f + std::sin(u * PI) * .19f + side * .07f * std::sin(v * PI) * (1 - u),
                           (.5f - v) * width * 2 - u * .33f};
        };
        auto tri = [&](Vector3 a, Vector3 b, Vector3 c) {
            Vector3 normal =
                Vector3Normalize(Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a)));
            for (Vector3 p : {a, b, c}) {
                m.vertices[n * 3] = p.x;
                m.vertices[n * 3 + 1] = p.y;
                m.vertices[n * 3 + 2] = p.z;
                Vector3 smooth = Vector3Normalize(
                    {-.23f * std::cos((p.x - .3f) / 1.15f * PI), normal.y > 0 ? 1.f : -1.f, .10f});
                m.normals[n * 3] = smooth.x;
                m.normals[n * 3 + 1] = smooth.y;
                m.normals[n * 3 + 2] = smooth.z;
                m.texcoords[n * 2] = p.x;
                m.texcoords[n * 2 + 1] = p.z;
                ++n;
            }
        };
        for (int i = 0; i < nx; ++i)
            for (int j = 0; j < nz; ++j)
                for (int side : {-1, 1}) {
                    float u = i / float(nx), v = j / float(nz), du = 1.f / nx, dv = 1.f / nz;
                    Vector3 a = point(u, v, side), b = point(u + du, v, side),
                            c = point(u + du, v + dv, side), d = point(u, v + dv, side);
                    if (side > 0) {
                        tri(a, b, c);
                        tri(a, c, d);
                    } else {
                        tri(a, c, b);
                        tri(a, d, c);
                    }
                }
        UploadMesh(&m, false);
        return m;
    }
    Mesh pinMesh() {
        const float heights[] = {0, .08f, .28f, .58f, .85f, 1.12f, 1.38f, 1.55f, 1.72f, 1.92f, 2.05f},
                    radii[] = {.22f, .31f, .38f, .40f, .34f, .22f, .14f, .15f, .23f, .23f, .05f};
        constexpr int rings = 11, sides = 20;
        Mesh m{};
        m.vertexCount = (rings - 1) * sides * 6;
        m.triangleCount = m.vertexCount / 3;
        m.vertices = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
        m.normals = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
        m.texcoords = (float *)MemAlloc(m.vertexCount * 2 * sizeof(float));
        int index = 0;
        auto put = [&](int r, int s) {
            float a = 2 * PI * s / sides,
                  slope = r == 0 ? 0 : (radii[r] - radii[r - 1]) / (heights[r] - heights[r - 1]);
            Vector3 n = Vector3Normalize({std::cos(a), -slope, std::sin(a)});
            m.vertices[index * 3] = std::cos(a) * radii[r];
            m.vertices[index * 3 + 1] = heights[r];
            m.vertices[index * 3 + 2] = std::sin(a) * radii[r];
            m.normals[index * 3] = n.x;
            m.normals[index * 3 + 1] = n.y;
            m.normals[index * 3 + 2] = n.z;
            m.texcoords[index * 2] = float(s) / sides;
            m.texcoords[index * 2 + 1] = heights[r] / 2;
            ++index;
        };
        for (int r = 0; r < rings - 1; ++r)
            for (int s = 0; s < sides; ++s) {
                put(r, s);
                put(r + 1, s);
                put(r + 1, s + 1);
                put(r, s);
                put(r + 1, s + 1);
                put(r, s + 1);
            }
        UploadMesh(&m, false);
        return m;
    }
    void init() {
        const char *vs = R"(#version 330
in vec2 vertexTexCoord;out vec2 uv;in vec3 vertexPosition;in vec3 vertexNormal;in vec4 vertexColor;uniform mat4 mvp;uniform mat4 matModel;uniform mat4 matNormal;out vec3 normal;out vec3 world;out vec4 color;
void main(){uv=vertexTexCoord;normal=normalize(mat3(matNormal)*vertexNormal);world=vec3(matModel*vec4(vertexPosition,1));color=vertexColor;gl_Position=mvp*vec4(vertexPosition,1);})";
        std::string path = std::string(GetApplicationDirectory()) + "assets/lighting.fs";
        char *fs = LoadFileText(path.c_str());
        if (!fs)
            throw std::runtime_error("Missing assets/lighting.fs beside executable");
        shader = LoadShaderFromMemory(vs, fs);
        UnloadFileText(fs);
        cube = LoadModelFromMesh(GenMeshCube(1, 1, 1));
        sphere = LoadModelFromMesh(GenMeshSphere(1, 24, 20));
        cylinder = LoadModelFromMesh(GenMeshCylinder(1, 1, 16));
        pin = LoadModelFromMesh(pinMesh());
        wing = LoadModelFromMesh(wingMesh());
        tube = LoadModelFromMesh(tubeMesh());
        course = LoadModelFromMesh(courseMesh());
        for (Model *m : {&cube, &sphere, &cylinder, &pin, &wing, &tube, &course})
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
    void updateScoreboard(const bowling::Match &match) {
        std::string key =
            std::to_string(arena) + ":" + std::to_string(mascot) + ":" + std::to_string(match.player);
        for (const auto &game : match.games) {
            key += "/" + std::to_string(game.frame);
            for (const auto &frame : game.frames) {
                key += "|";
                for (int roll : frame)
                    key += std::to_string(roll) + ",";
            }
        }
        if (key == scoreboardKey)
            return;
        scoreboardKey = key;
        bool manual = !mascot && (arena == 0 || arena == 1);
        Color paint = manual ? Color{58, 77, 51, 255} : Color{9, 13, 14, 255};
        Color letters = manual ? Color{231, 232, 207, 255} : Color{249, 195, 70, 255};
        auto total = [](const bowling::Game &game) {
            int value = 0;
            for (auto v : game.scores())
                if (v)
                    value = *v;
            return value;
        };
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
            center(std::to_string(match.games[match.player].standing()), 512, 245, 35, letters);
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
                center(std::to_string(total(match.games[side])), x + 166, 172, 85, WHITE);
                center("FRAME " + std::to_string(std::min(10, match.games[side].frame + 1)), x + 166,
                       274, 18, cream);
            }
            letters = WHITE;
        }
        // These frame scores are live game data, not invented real-world MLB results.
        text("BOWLING", 25, 351, 17, letters);
        for (int frame = 0; frame < 10; ++frame)
            center(std::to_string(frame + 1), 245 + frame * 60, 349, 17, letters);
        center("TOTAL", 945, 349, 16, letters);
        for (int player = 0; player < 2; ++player) {
            const auto &game = match.games[player];
            auto scores = game.scores();
            float y = 377 + player * 45.f;
            text(player ? "RIVAL" : "YOU", 28, y + 6, 24, letters);
            for (int frame = 0; frame < 10; ++frame) {
                float x = 217 + frame * 60;
                DrawRectangleLinesEx({x, y, 56, 37}, 1, Fade(letters, .55f));
                if (frame == game.frame && player == match.player)
                    DrawRectangleLinesEx({x, y, 56, 37}, 3, letters);
                center(scores[frame] ? std::to_string(*scores[frame]) : "-", x + 28, y + 8, 23, letters);
            }
            center(std::to_string(total(game)), 945, y + 4, 30, letters);
        }
        DrawRectangle(0, 476, 1024, 36, {5, 8, 9, 255});
        std::string welcome =
            mascot ? "RAYS UP!" : std::string("WELCOME TO ") + bowling::arenas[arena].name;
        float size = std::min(28.f, 960.f / std::max(1.f, tw(welcome, 28)) * 28);
        center(welcome, 512, 480, size, cream);
        EndTextureMode();
        GenTextureMipmaps(&scoreboard.texture);
        SetTextureFilter(scoreboard.texture, TEXTURE_FILTER_TRILINEAR);
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
    void makeSkins() {
        const Color palettes[] = {{20, 46, 88, 255},  {255, 196, 43, 255}, {48, 210, 177, 255},
                                  {212, 49, 46, 255}, {120, 60, 194, 255}, {103, 201, 249, 255},
                                  {37, 151, 83, 255}, {236, 224, 207, 255}};
        for (int id = 0; id < 64; ++id) {
            Image im = GenImageColor(256, 128, palettes[id % 8]);
            Color base = id < 60 ? GetColor(teams[id % 30].color)
                                 : palettes[std::array<int, 4>{0, 2, 5, 1}[id - 60]];
            for (int y = 0; y < 128; ++y)
                for (int x = 0; x < 256; ++x) {
                    float u = x / 256.f, v = y / 128.f;
                    bool mark = false;
                    switch (id < 30 ? 0 : id < 60 ? 1 + (id - 30) % 7 : id - 56) {
                    case 0:
                        mark =
                            std::sin(u * 32 + std::sin(v * 19) * 3 + std::sin(u * 12 + v * 16)) > .65f;
                        break;
                    case 1:
                        mark = std::fmod(u * 7 + v * .6f + std::abs(std::sin(v * 28)) * .3f, 1.f) < .11f;
                        break;
                    case 2:
                        mark = std::abs(std::sin(u * PI * 4)) < .15f && int(v * 35) % 2 == 0;
                        break;
                    case 3: {
                        float a = std::fmod(u * 8, 1.f) - .5f, b = std::fmod(v * 4, 1.f) - .5f;
                        mark = (std::abs(a) < .05f || std::abs(b) < .05f) && a * a + b * b < .12f;
                        break;
                    }
                    case 4:
                        mark = std::sin(u * 54 + std::sin(v * 13) * 4) > .25f;
                        break;
                    case 5:
                        mark = std::sin(v * 60 + std::sin(u * 12) * 5) > .7f;
                        break;
                    case 6:
                        mark = (int(u * 16) + int(v * 8)) % 2;
                        break;
                    case 7:
                        mark = ((x * 73 + y * 97 + x * y * 11) % 997) < 24;
                        break;
                    }
                    if (id == 60)
                        mark = std::sin(std::atan2(v - .5f, u - .5f) * 12) > .25f;
                    if (id == 61)
                        mark = std::abs(u - .5f) * 1.2f + std::abs(v - .5f) < .28f;
                    if (id == 62)
                        mark = std::sin(v * 55 + std::sin(u * 15) * 4) > .4f;
                    if (id == 63)
                        mark = (std::sin(u * 48) * std::sin(v * 48) > .6f) ||
                               std::hypot(u - .5f, v - .5f) < .12f;
                    Color c =
                        mark ? (id / 8 == 4 ? Color{15, 18, 30, 255} : Color{251, 223, 137, 255}) : base;
                    float glow = .8f + .2f * std::sin(u * 6 + v * 4);
                    c.r = static_cast<unsigned char>(c.r * glow);
                    c.g = static_cast<unsigned char>(c.g * glow);
                    c.b = static_cast<unsigned char>(c.b * glow);
                    ImageDrawPixel(&im, x, y, c);
                }
            ImageDrawText(&im, id < 60 ? teams[id % 30].id : "TB", 90, 45, 32, id >= 60 ? gold : WHITE);
            skins[id] = LoadTextureFromImage(im);
            GenTextureMipmaps(&skins[id]);
            SetTextureFilter(skins[id], TEXTURE_FILTER_TRILINEAR);
            UnloadImage(im);
        }
    }
    void bowlingBall(Vector3 p, float roll, float radius = .51f) {
        rlPushMatrix();
        rlTranslatef(p.x, p.y, p.z);
        rlRotatef(roll, 1, 0, 0);
        Texture2D original = sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture;
        sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skins[skin];
        orb({0, 0, 0}, {radius, radius, radius}, WHITE);
        sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = original;
        for (Vector3 hole :
             {Vector3{-.12f, .12f, -.47f}, Vector3{.12f, .12f, -.47f}, Vector3{0, -.12f, -.48f}})
            orb(Vector3Scale(hole, radius / .51f), {.065f * radius / .51f, .065f * radius / .51f, .025f},
                ink);
        rlPopMatrix();
    }
    void drawPin(const bowling::Body &p, Color team, float time) {
        rlPushMatrix();
        rlTranslatef(geo::renderX(p.x), .06f, p.z);
        rlScalef(geo::pinRadius / .40f, geo::pinHeight / 2.05f, geo::pinRadius / .40f);
        rlRotatef(p.down ? p.fall * 86 : std::sin(time * 3 + p.id) * 2, p.id % 2 ? 1.f : -1.f, 0, .4f);
        DrawModel(pin, {0, 0, 0}, 1, {250, 242, 220, 255});
        cyl({0, 1.35f, 0}, .16f, .10f, red);
        cyl({0, 1.48f, 0}, .16f, .06f, red);
        orb({0, 1.96f, 0}, {.29f, .16f, .28f}, team);
        box({0, 1.91f, -.22f}, {.58f, .07f, .36f}, team);
        for (float x : {-.105f, .105f}) {
            orb({x, 1.77f, -.205f}, {.075f, .09f, .035f}, WHITE);
            orb({x, 1.77f, -.239f}, {.032f, .048f, .018f}, ink);
            beam({x - .06f, 1.88f, -.23f}, {x + .05f, 1.90f, -.23f}, .017f, ink);
        }
        orb({0, .85f, -.30f}, {.20f, .23f, .055f}, team);
        for (int j = 0; j < 7; ++j)
            orb({0, .64f + j * .055f, -.365f}, {.012f, .015f, .009f}, cream);
        box({0, 1.62f, -.19f}, {.13f, .035f, .045f}, ink);
        for (int side : {-1, 1}) {
            orb({side * .39f, .75f, -.02f}, {.13f, .19f, .15f}, {158, 91, 44, 255});
            orb({side * .15f, .08f, -.12f}, {.22f, .10f, .32f}, team);
        }
        rlPopMatrix();
    }
    void rider(Vector3 p, float time, bool ready = false, float bank = 0) {
        rlPushMatrix();
        rlTranslatef(p.x, p.y + std::sin(time * 3) * .035f, p.z);
        rlRotatef(std::sin(time * 3) * 2 - bank * 3.5f, 0, 0, 1);
        rlRotatef(bank * 2, 0, 1, 0);
        // Real ring silhouette, two-tone rubber, stitched grab rope and handles.
        DrawModelEx(tube, {0, -.16f, 0}, {1, 0, 0}, 90, {1.05f, 1.20f, .95f}, {18, 52, 103, 255});
        DrawModelEx(tube, {0, -.03f, 0}, {1, 0, 0}, 90, {1.01f, 1.16f, .60f}, {55, 146, 194, 255});
        orb({0, -.16f, 0}, {.80f, .12f, .94f}, {9, 28, 58, 255});
        for (int i = 0; i < 48; ++i) {
            float a = i * 2 * PI / 48, b = (i + 1) * 2 * PI / 48;
            beam({std::cos(a) * 1.18f, .04f, std::sin(a) * 1.36f},
                 {std::cos(b) * 1.18f, .04f, std::sin(b) * 1.36f}, .022f, cream);
        }
        for (int side : {-1, 1}) {
            box({side * .96f, .16f, 0}, {.12f, .08f, .37f}, ink);
            for (float z : {-.20f, .20f})
                orb({side * .96f, .12f, z}, {.11f, .07f, .08f}, gold);
        }
        orb({0, .19f, 0}, {.58f, .50f, .69f}, {31, 155, 176, 255});
        orb({0, .16f, -.30f}, {.49f, .39f, .42f}, cream);
        box({0, .20f, -.704f}, {.10f, .31f, .025f}, {18, 58, 97, 255});
        box({-.10f, .35f, -.69f}, {.22f, .07f, .025f}, {18, 58, 97, 255});
        for (int side : {-1, 1}) {
            rlPushMatrix();
            if (side < 0)
                rlScalef(-1, 1, 1);
            rlDisableBackfaceCulling();
            rlRotatef(10 + std::sin(time * 6) * 7, 0, 0, 1);
            DrawModel(wing, {0, .05f, 0}, 1.08f, {32, 161, 182, 255});
            rlEnableBackfaceCulling();
            rlPopMatrix();
        }
        rlPushMatrix();
        rlTranslatef(0, .50f, .1f);
        if (ready)
            rlRotatef(155 + std::sin(time) * 8, 0, 1, 0);
        orb({0, .05f, .03f}, {.45f, .31f, .46f}, {40, 164, 182, 255});
        orb({0, -.06f, .32f}, {.34f, .15f, .24f}, {173, 222, 215, 255});
        for (float x : {-.19f, .19f}) {
            float blink = std::fmod(time + 1, 5) < .13f ? .03f : .14f;
            orb({x, .19f, .36f}, {.145f, blink, .10f}, WHITE);
            orb({x, .20f, .44f}, {.068f, blink * .8f, .03f}, {20, 48, 68, 255});
            orb({x - .02f, .24f, .465f}, {.023f, .027f, .01f}, WHITE);
            beam({x - .10f, .36f, .35f}, {x + .08f, .34f, .35f}, .022f, ink);
        }
        for (int i = 0; i < 8; ++i) {
            float a = PI * .15f + i * PI * .7f / 7;
            orb({std::cos(a) * .20f, -.04f - std::sin(a) * .055f, .53f}, {.04f, .018f, .015f}, ink);
        }
        orb({0, .38f, 0}, {.47f, .18f, .44f}, {15, 42, 79, 255});
        orb({0, .34f, .38f}, {.50f, .055f, .34f}, {19, 56, 94, 255});
        orb({0, .56f, 0}, {.055f, .035f, .055f}, gold);
        box({0, .43f, .41f}, {.09f, .15f, .02f}, cream);
        box({0, .49f, .41f}, {.23f, .04f, .02f}, cream);
        rlPopMatrix();
        Vector3 last{0, 0, -.5f};
        for (int i = 1; i <= 10; ++i) {
            float t = i / 10.f;
            Vector3 next{std::sin(time * 3 + t * 3) * t * .30f, -.04f - t * .1f, -.5f - t * 1.2f};
            DrawCylinderEx(last, next, .12f * (1 - t) + .015f, .1f * (1 - t) + .008f, 10,
                           {27, 129, 153, 255});
            last = next;
        }
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
                    orb({side * 7.1f, 4.4f + .3f * std::sin(time + i), z}, {.65f, .12f, .6f},
                        {30, 70, 121, 255});
                    for (int d : {-1, 1})
                        orb({side * 7.1f + d * .65f, 4.4f + .3f * std::sin(time + i), z - .1f},
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
               Camera3D cam, float time) {
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
        BeginMode3D(cam);
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
        for (const auto &p : sim.pins) {
            auto visible = p;
            visible.z = p.z;
            drawPin(visible, team, time);
        }
        if (ballPos.z < geo::surfaceEnd) {
            if (mascot)
                rider({ballPos.x + (aim ? shot.aim * 1.6f : 0), .57f,
                       ballPos.z - (aim ? slingPower * 2.7f : 0)},
                      time, aim, sim.active ? sim.ball.vx : 0);
            else
                bowlingBall({ballPos.x, sim.gutter ? .28f : (sim.hasBall ? sim.ball.r : geo::ballRadius),
                             ballPos.z},
                            sim.hasBall ? ballPos.z * 110 : time * 12,
                            sim.hasBall ? sim.ball.r : geo::ballRadius);
        }
        if (aim && mascot) {
            float pull = slingPower * 2.7f;
            Vector3 center{ballPos.x + shot.aim * 1.6f, .70f, ballPos.z - pull};
            for (int side : {-1, 1}) {
                cyl({side * 4.8f, .04f, 2.4f}, .24f, 1.3f, {24, 69, 110, 255});
                orb({side * 4.8f, 1.4f, 2.4f}, {.31f, .34f, .31f}, gold);
                beam({side * 4.8f, 1.15f, 2.4f}, center, .095f, {220, 44, 60, 255});
            }
        }
        if (aim)
            for (int i = 0; i < 20; ++i) {
                float t = i * .07f, x = geo::renderX(shot.position + std::sin(shot.aim * .16f) * 22 * t +
                                                     .5f * shot.curve * .85f * t * t);
                if (std::abs(x) < 4.7f)
                    orb({x, .07f, geo::foulLine + t * 22}, {.045f, .015f, .10f}, gold);
            }
        EndMode3D();
    }
    void close() {
        for (Model m : {cube, sphere, cylinder, pin, wing, tube, course})
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
