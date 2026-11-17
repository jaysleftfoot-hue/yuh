#pragma once
#include "campaign.hpp"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "ui.hpp"
struct Art {
    Model cube{}, sphere{}, cylinder{}, pin{}, wing{};
    Shader shader{};
    std::array<Texture2D, 64> skins{};
    std::array<RenderTexture2D, 10> signs{};
    RenderTexture2D ad{};
    int arena = 0, skin = 0;
    bool mascot = false;
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
                m.normals[n * 3] = normal.x;
                m.normals[n * 3 + 1] = normal.y;
                m.normals[n * 3 + 2] = normal.z;
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
        for (Model *m : {&cube, &sphere, &cylinder, &pin, &wing})
            m->materials[0].shader = shader;
    }
    void box(Vector3 p, Vector3 scale, Color c) {
        DrawModelEx(cube, p, {0, 1, 0}, 0, scale, c);
    }
    void orb(Vector3 p, Vector3 scale, Color c) {
        DrawModelEx(sphere, p, {0, 1, 0}, 0, scale, c);
    }
    void cyl(Vector3 p, float r, float h, Color c) {
        DrawModelEx(cylinder, p, {0, 1, 0}, 0, {r, h, r}, c);
    }
    void beam(Vector3 a, Vector3 b, float r, Color c) {
        DrawCylinderEx(a, b, r, r, 8, c);
    }
    void material(int kind) {
        SetShaderValue(shader, GetShaderLocation(shader, "woodMode"), &kind, SHADER_UNIFORM_INT);
    }
    void makeSigns() {
        for (int a = 0; a < 10; ++a) {
            signs[a] = LoadRenderTexture(1024, 256);
            BeginTextureMode(signs[a]);
            ClearBackground({7, 19, 38, 255});
            DrawRectangleGradientV(0, 0, 1024, 256, {24, 62, 90, 255}, {5, 13, 28, 255});
            for (int i = 0; i < 1024; i += 8)
                DrawLine(i, 0, i, 256, {28, 70, 86, 90});
            DrawRectangleLinesEx({8, 8, 1008, 240}, 5, gold);
            DrawRectangleLinesEx({17, 17, 990, 222}, 2, aqua);
            float size = std::min(69.f, 900.f / std::max(1.f, tw(bowling::arenas[a].name, 69)) * 69);
            center(bowling::arenas[a].name, 512, 50, size, cream);
            center(bowling::arenas[a].theme, 512, 143, 36, aqua);
            center("B I G   L E A G U E   B O W L I N G", 512, 205, 17, gold);
            EndTextureMode();
            SetTextureFilter(signs[a].texture, TEXTURE_FILTER_BILINEAR);
        }
        ad = LoadRenderTexture(512, 256);
        BeginTextureMode(ad);
        ClearBackground({11, 40, 64, 255});
        DrawRectangleLinesEx({8, 8, 496, 240}, 5, gold);
        center("RAYS UP!", 256, 34, 66, gold);
        center("SUNSHINE. STRIKES.", 256, 124, 27, cream);
        center("BALLPARK MAYHEM", 256, 177, 26, aqua);
        EndTextureMode();
        SetTextureFilter(ad.texture, TEXTURE_FILTER_BILINEAR);
    }
    void sign(Texture2D t, Vector3 p, float w, float h) {
        rlSetTexture(t.id);
        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);
        rlNormal3f(0, 0, -1);
        rlTexCoord2f(1, 0);
        rlVertex3f(p.x - w / 2, p.y - h / 2, p.z);
        rlTexCoord2f(1, 1);
        rlVertex3f(p.x - w / 2, p.y + h / 2, p.z);
        rlTexCoord2f(0, 1);
        rlVertex3f(p.x + w / 2, p.y + h / 2, p.z);
        rlTexCoord2f(0, 0);
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
            orb(hole, {.065f, .065f, .025f}, ink);
        rlPopMatrix();
    }
    void drawPin(const bowling::Body &p, Color team, float time) {
        rlPushMatrix();
        rlTranslatef(p.x - 5, .06f, p.z);
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
    void rider(Vector3 p, float time, bool ready = false) {
        rlPushMatrix();
        rlTranslatef(p.x, p.y + std::sin(time * 3) * .035f, p.z);
        orb({0, -.25f, 0}, {.88f, .22f, 1.13f}, {212, 123, 43, 255});
        orb({0, -.12f, 0}, {.77f, .13f, 1.02f}, cream);
        for (float x : {-.53f, .53f}) {
            beam({x, -.38f, -.85f}, {x, -.38f, .85f}, .065f, {137, 189, 207, 255});
            for (int i = 0; i < 12; ++i)
                orb({x, -.04f, -.85f + i * .15f}, {.04f, .015f, .025f}, red);
        }
        orb({0, .19f, 0}, {.58f, .50f, .69f}, {31, 155, 176, 255});
        orb({0, .16f, -.30f}, {.49f, .39f, .42f}, cream);
        box({0, .20f, -.704f}, {.10f, .31f, .025f}, {18, 58, 97, 255});
        box({-.10f, .35f, -.69f}, {.22f, .07f, .025f}, {18, 58, 97, 255});
        for (int side : {-1, 1}) {
            rlPushMatrix();
            if (side < 0)
                rlRotatef(180, 0, 1, 0);
            rlRotatef(10 + std::sin(time * 6) * 7, 0, 0, 1);
            DrawModel(wing, {0, 0, 0}, 1, {32, 161, 182, 255});
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
            for (int ring = 0; ring < 5; ++ring) {
                float z = -2 + ring * 10.f;
                Vector3 last{-25, 6, z};
                for (int i = 1; i <= 32; ++i) {
                    float a = PI * i / 32;
                    Vector3 next{-25 * std::cos(a), 6 + 14 * std::sin(a), z};
                    beam(last, next, .12f, cream);
                    last = next;
                }
            }
            box({0, 21, 17}, {56, .3f, 51}, {128, 165, 190, 255});
            box({9, 1, 19}, {5, 2, 16}, {31, 104, 149, 255});
            box({9, 2.05f, 19}, {4.5f, .08f, 15.5f}, aqua);
            for (int i = 0; i < 5; ++i)
                orb({9 + std::sin(time + i) * 1.5f, 2.2f, 13 + i * 3.f}, {.65f, .06f, .4f},
                    {24, 75, 108, 255});
        }
    }
    void world(const bowling::Simulation &sim, const bowling::Shot &shot, bool aim, int selected,
               Camera3D cam, float time) {
        Color team = GetColor(teams[selected].color);
        SetShaderValue(shader, GetShaderLocation(shader, "eye"), &cam.position, SHADER_UNIFORM_VEC3);
        Vector3 shadow[10];
        for (int i = 0; i < 10; ++i)
            shadow[i] = {1000, 0, 1000};
        for (size_t i = 0; i < sim.pins.size(); ++i)
            shadow[i] = {sim.pins[i].x - 5, 0, sim.pins[i].z};
        SetShaderValueV(shader, GetShaderLocation(shader, "pinShadows"), shadow, SHADER_UNIFORM_VEC3,
                        10);
        Vector3 ballPos{(sim.hasBall ? sim.ball.x : shot.position) - 5, 0, sim.hasBall ? sim.ball.z : 2};
        SetShaderValue(shader, GetShaderLocation(shader, "ballShadow"), &ballPos, SHADER_UNIFORM_VEC3);
        BeginMode3D(cam);
        material(0);
        box({0, -.7f, 15}, {95, 1, 95}, {17, 39, 43, 255});
        for (int side : {-1, 1})
            for (int lane = 0; lane < 2; ++lane) {
                float x = side * (9.4f + lane * 7.4f);
                material(1);
                box({x, -.03f, 14}, {6.3f, .18f, 32}, {205, 158, 94, 255});
                material(0);
                for (int edge : {-1, 1}) {
                    box({x + edge * 3.3f, -.10f, 14}, {.35f, .13f, 32}, {21, 39, 57, 255});
                    box({x + edge * 3.65f, .07f, 14}, {.16f, .3f, 32}, {20, 116, 151, 255});
                }
                for (int row = 0; row < 4; ++row)
                    for (int col = 0; col <= row; ++col)
                        DrawModel(pin, {x + (col - row * .5f) * .7f, .04f, 25 + row * .65f}, .7f, cream);
            }
        material(1);
        box({0, -.06f, 13.5f}, {10, .2f, 32}, {222, 171, 103, 255});
        material(0);
        for (float x : {-5.35f, 5.35f}) {
            box({x, -.08f, 13.5f}, {.64f, .15f, 32}, {16, 37, 55, 255});
            for (float d : {-.30f, .30f})
                beam({x + d, .10f, -2}, {x + d, .10f, 30}, .07f, {107, 156, 176, 255});
            box({x + (x < 0 ? -.5f : .5f), .12f, 13.5f}, {.25f, .42f, 32}, team);
            box({x + (x < 0 ? -.5f : .5f), .36f, 13.5f}, {.15f, .025f, 32}, aqua);
        }
        box({0, .047f, 2}, {10, .013f, .13f}, red);
        for (int i = -3; i <= 3; ++i) {
            float z = 7 + std::abs(i) * .32f;
            DrawTriangle3D({i * .95f - .12f, .051f, z}, {i * .95f, .051f, z + .32f},
                           {i * .95f + .12f, .051f, z}, ink);
        }
        for (int i = -2; i <= 2; ++i)
            DrawSphere({i * 1.1f, .05f, 3.3f}, .045f, ink);
        box({0, 2, 30}, {46, 4.1f, 1.8f}, {8, 17, 28, 255});
        for (int i = -3; i <= 3; ++i) {
            float x = i * 7.2f;
            box({x, 1.35f, 28.98f}, {6.9f, 2.7f, .15f}, {2, 8, 13, 255});
            box({x, 2.85f, 28.7f}, {7, .20f, .35f}, team);
            box({x, 2.71f, 28.65f}, {6.5f, .06f, .12f}, aqua);
        }
        box({0, 5.5f, 30.5f}, {18, 3.9f, 1}, ink);
        box({0, 7.65f, 30.5f}, {19, .25f, 1.4f}, gold);
        sign(signs[arena].texture, {0, 5.5f, 29.95f}, 17.6f, 3.7f);
        for (float x : {-15.f, 15.f})
            sign(arena == 9 ? ad.texture : signs[arena].texture, {x, 5.4f, 29.8f}, 7, 3.5f);
        material(2);
        box({0, 9, 39}, {66, 18, 1}, {91, 62, 48, 255});
        material(0);
        for (int i = -4; i <= 4; ++i) {
            float x = i * 7.f;
            box({x, 9.8f, 38.4f}, {3.9f, 6, .08f}, i % 2 ? team : Color{29, 92, 119, 255});
            box({x, 6.7f, 38.3f}, {4.1f, .12f, .15f}, gold);
        }
        for (int side : {-1, 1}) {
            material(2);
            box({side * 23.f, 2.6f, 15}, {1.2f, 5.2f, 43}, {139, 69, 48, 255});
            material(0);
            box({side * 21.5f, 5.4f, 15}, {5, .38f, 43}, {92, 105, 111, 255});
            for (float z : {-4.f, 6.f, 16.f, 26.f, 36.f}) {
                box({side * 22.f, 8, z}, {.65f, 16, .65f}, {41, 72, 86, 255});
                beam({side * 19.f, 5.9f, z}, {side * 19.f, 7.1f, z}, .055f, cream);
            }
            beam({side * 19.f, 7.1f, -5}, {side * 19.f, 7.1f, 37}, .07f, cream);
            for (int row = 0; row < 4; ++row) {
                box({side * (25.f + row * 1.5f), 5.8f + row * .75f, 17}, {1.6f, 1, 42},
                    {32, 65, 91, 255});
                for (int seat = 0; seat < 20; ++seat) {
                    float x = side * (25.f + row * 1.5f), y = 6.5f + row * .75f, z = -2 + seat * 2;
                    Color shirt = (seat + row) % 3 == 0 ? team : Color{181, 154, 98, 255};
                    orb({x, y, z}, {.24f, .33f, .18f}, shirt);
                    orb({x, y + .43f, z}, {.17f, .20f, .17f}, {220, 169, 121, 255});
                }
            }
            for (float z : {-1.f, 10.f, 21.f}) {
                float x = side * 20.f;
                cyl({x, 0, z}, .12f, 1.3f, {86, 101, 111, 255});
                cyl({x, 1.3f, z}, .85f, .13f, {151, 78, 45, 255});
                cyl({x - .35f, 1.45f, z}, .13f, .35f, red);
                cyl({x - .35f, 1.8f, z}, .15f, .035f, WHITE);
                beam({x - .35f, 1.8f, z}, {x - .3f, 2.05f, z}, .012f, cream);
                orb({x + .3f, 1.58f, z}, {.13f, .13f, .13f}, cream);
            }
        }
        for (float z : {-3.f, 9.f, 21.f, 33.f}) {
            beam({-23, 12, z}, {23, 12, z}, .14f, {89, 123, 146, 255});
            beam({-23, 13.3f, z}, {23, 13.3f, z}, .11f, {89, 123, 146, 255});
            for (int x = -22; x < 22; x += 4)
                beam({float(x), 12, z}, {float(x + 2), 13.3f, z}, .065f, {115, 145, 160, 255});
            for (float x : {-12.f, 0.f, 12.f}) {
                box({x, 11.7f, z}, {5, .2f, 1.6f}, ink);
                box({x, 11.55f, z}, {4.6f, .035f, 1.35f}, {245, 232, 190, 255});
            }
        }
        box({0, 16, 17}, {70, .45f, 52},
            arena == 9 ? Color{126, 164, 185, 255} : Color{51, 72, 91, 255});
        rlPushMatrix();
        rlTranslatef(0, 7, 18);
        rlScalef(1.5f, .6f, .65f);
        landmarks(time);
        rlPopMatrix();
        for (const auto &p : sim.pins)
            drawPin(p, team, time);
        if (ballPos.z < 31) {
            if (mascot)
                rider({ballPos.x, .57f, ballPos.z}, time, aim);
            else
                bowlingBall({ballPos.x, sim.gutter ? .28f : .6f, ballPos.z},
                            sim.hasBall ? ballPos.z * 110 : time * 12, sim.hasBall ? sim.ball.r : .51f);
        }
        if (aim)
            for (int i = 0; i < 20; ++i) {
                float t = i * .07f, x = shot.position - 5 + std::sin(shot.aim * .16f) * 22 * t +
                                        .5f * shot.curve * .85f * t * t;
                if (std::abs(x) < 4.7f)
                    orb({x, .07f, 2 + t * 22}, {.045f, .015f, .10f}, gold);
            }
        EndMode3D();
    }
    void close() {
        for (Model m : {cube, sphere, cylinder, pin, wing})
            UnloadModel(m);
        for (auto t : skins)
            UnloadTexture(t);
        for (auto s : signs)
            UnloadRenderTexture(s);
        UnloadRenderTexture(ad);
        UnloadShader(shader);
    }
};
