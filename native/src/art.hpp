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
    bowling::PinsetterCycle setter;
    float strikeSalute = 0; // 0..1 during the live post-strike sweep
    float crowdCheer = 0;
    Texture2D strikePennant{}, shop{};
    bool returningBall = false;
    Texture2D gearCase{}, tank{}, kelp{}, reef{}, spray{}, sunburst{}, tokenMedal{};
    std::array<Texture2D, 10> outfield{};
    Texture2D manualBoard{}, electricBoard{}, scorePlates{}, enamelLettering{}, pinGauge{};
    std::array<Texture2D, 2> scoreRails{};
    std::array<Texture2D, 3> reactions{};
    std::array<Texture2D, bowling::powerCount> badges{};
    std::array<Texture2D, 2> rideBadges{};
    void loadBallparkArt() {
        auto load = [](const std::string &name) {
            const auto path = std::string(GetApplicationDirectory()) + "assets/ballpark-art/" + name;
            Texture2D texture = LoadTexture(path.c_str());
            if (!texture.id)
                throw std::runtime_error("Missing ballpark art: " + name);
            GenTextureMipmaps(&texture);
            SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
            return texture;
        };
        shop = load("pro-shop.jpg");
        strikePennant = load("strike-pennant.png");
        manualBoard = load("manual-board.jpg");
        electricBoard = load("electric-board.jpg");
        enamelLettering = load("enamel-lettering.png");
        scorePlates = load("score-plates.png");
        pinGauge = load("pin-gauge.png");
        scoreRails = {load("score-rail-home.png"), load("score-rail-away.png")};
        gearCase = load("equipment-case.jpg");
        constexpr const char *murals[]{"wrigley", "fenway", "oracle", "pnc", "coors", "kauffman",
                                       "chase", "seattle", "yankee", "trop"};
        for (size_t i = 0; i < outfield.size(); ++i)
            outfield[i] = load(std::string(murals[i]) + ".jpg");
        tank = load("tank.jpg");
        kelp = load("kelp-gallery.jpg");
        reef = load("sunken-ballpark.jpg");
        spray = load("wave-spray.png");
        SetTextureWrap(tank, TEXTURE_WRAP_MIRROR_REPEAT);
        course.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tank;
        sunburst = load("sunburst.jpg");
        reactions = {load("slugger.png"), load("umpire.png"), load("ballboy.png")};
        tokenMedal = load("token.png");
        rideBadges = {load("wave-crash.png"), load("double-pins.png")};
        for (int i = 0; i < bowling::powerCount; ++i)
            badges[i] = load("power-" + std::to_string(i) + ".png");
    }
    std::array<Texture2D, 64> skins{};
    RenderTexture2D scoreboard{};
    std::array<Texture2D, 10> parkLogos{}, teamLogos{};
    std::string scoreboardKey;
    int arena = 0, skin = 0;
    bowling::MascotPose mascotPose;
    const bowling::PowerPickup *powerPickup = nullptr;
    const bowling::MascotTokens *mascotTokens = nullptr;
    const bowling::RidePickups *ridePickups = nullptr;
    std::optional<bowling::RidePickups::WaveImpact> wave;
    bowling::StadiumCue cue = bowling::StadiumCue::None;
    float cueSeconds = 0, tileSeconds = 0;
    using Tiles = std::array<std::array<std::string, 10>, 2>;
    Tiles visibleTiles{}, oldTiles{};
    std::array<int, 2> visibleTotals{}, oldTotals{};
    void enamelLabel(int row, Rectangle dest) const {
        const float h = enamelLettering.height / 4.f;
        DrawTexturePro(enamelLettering, {0, row * h, float(enamelLettering.width), h},
                       dest, {0, 0}, 0, WHITE);
    }
    void plateTotal(int total, int before, float cx) const {
        const float width = scorePlates.width / 11.f;
        const float phase = 1.f - tileSeconds / .42f;
        for (int place = 2, scale = 100; place >= 0; --place, scale /= 10) {
            // Sprite 10 is the empty enamel plate for a leading zero.
            const int digit = place && total < scale ? 10 : total / scale % 10;
            const int old = place && before < scale ? 10 : before / scale % 10;
            const bool moving = tileSeconds > 0 && digit != old;
            const float angle = moving ? std::abs(std::cos(PI * phase)) : 1.f;
            const int face = moving && phase < .5f ? old : digit;
            const float left = cx - 116 + (2 - place) * 78;
            if (moving)
                DrawTexturePro(scorePlates, {10 * width, 0, width, float(scorePlates.height)},
                               {left, 128, 77, 126}, {0, 0}, 0, {112, 123, 114, 255});
            DrawTexturePro(scorePlates, {face * width, 0, width, float(scorePlates.height)},
                           {left, 128 + 63 * (1 - angle), 77, 126 * angle}, {0, 0}, 0,
                           ColorBrightness(WHITE, -.35f * (1.f - angle)));
        }
    }
    bool mascot = false, inSceneryTransform = false;
    Mesh courseMesh() {
        constexpr int rings = 26 + int(geo::mascotLane.sceneryExtension() / 2) + 1, sides = 24;
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
                m.texcoords[n * 2] = v.z * .042f;
                m.texcoords[n * 2 + 1] = 1.f - v.y * .093f;
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
uniform float capTip;
uniform vec3 finLoad; // draw, signed turn and launch recoil
void main(){
localPos=vertexPosition;
vec3 pos=vertexPosition;
vec3 norm=vertexNormal;
if(woodMode==6){
    // Cap crown, brim and button have their own painted UV islands on the GLB.
    bool crown=vertexTexCoord.x>=.55 && vertexTexCoord.y>=.645;
    bool trim=vertexTexCoord.x>=.499 && vertexTexCoord.x<=.506 &&
              vertexTexCoord.y>=.734 && vertexTexCoord.y<=.791;
    bool cap=crown || trim;
    if(cap){
        float angle=-.62*capTip;
        mat2 tilt=mat2(cos(angle),sin(angle),-sin(angle),cos(angle));
        pos.yz=tilt*(pos.yz-vec2(.85,-.18))+vec2(.85+.11*capTip,-.18);
        norm.yz=tilt*norm.yz;
    }
    float wing=pow(clamp((abs(pos.x)-.57)/1.46,0.0,1.0),2.0);
    float stroke=.21*sin(sceneTime*3.8+pos.z*2.3)-.31*finLoad.x
        +.55*sign(pos.x)*finLoad.y+.25*finLoad.z;
    pos.y+=wing*stroke;
    norm.x-=norm.y*2.0*sqrt(wing)*stroke*sign(pos.x)/1.46;
    float tail=clamp((-pos.z-.9)/1.7,0.0,1.0);
    pos.x+=tail*tail*(.22*sin(sceneTime*3.0+pos.z*2.0)-.44*finLoad.y);
}
uv=vertexTexCoord;normal=normalize(mat3(matNormal)*norm);world=vec3(matModel*vec4(pos,1));color=vertexColor;gl_Position=mvp*vec4(pos,1);})";
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
        const std::array<int, 2> totals{match.total(0), match.total(1)};
        key += ":" + std::to_string(totals[0]) + ":" + std::to_string(totals[1]);
        const bool flipped = nextTiles != visibleTiles || totals != visibleTotals;
        if (flipped) {
            oldTotals = visibleTotals;
            visibleTotals = totals;
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
        const bool manual = !mascot && (arena == 0 || arena == 1);
        const int park = mascot ? 9 : arena;
        const Color visitorInk = {255, 200, 91, 255}, homeInk = {151, 232, 207, 255};
        BeginTextureMode(scoreboard);
        ClearBackground(BLACK);
        sprite(manual ? manualBoard : electricBoard, {0, 0, 1024, 512});
        if (!replayFeed && cue != bowling::StadiumCue::Turkey) {
            if (manual) {
                enamelLabel(park, {322, 17, 380, 56});
            } else {
                auto logo = [](Texture2D texture, float x, float y, float width, float height) {
                    float scale = std::min(width / texture.width, height / texture.height);
                    DrawTextureEx(texture, {x - texture.width * scale / 2, y}, 0, scale, WHITE);
                };
                logo(parkLogos[park], 512, 68, 360, 65);
                logo(teamLogos[park], 512, 142, 155, 124);
            }
            for (int side = 0; side < 2; ++side) {
                const float cx = side ? 810 : 214;
                const Color lamp = side ? homeInk : visitorInk;
                if (manual) {
                    enamelLabel(2 + side, {cx - 188, 75, 376, 52});
                    plateTotal(visibleTotals[side], oldTotals[side], cx);
                } else {
                    arcadeCenter(side ? "HOME" : "VISITORS", cx, 75, 40, lamp);
                    scoreboardLights(std::to_string(visibleTotals[side]), cx, 142, 16, lamp);
                }
            }
            arcadeCenter("FRAME  " + std::to_string(std::min(10, match.games[match.player].frame + 1)),
                         247, 269, 28, cream);
            arcadeCenter(std::to_string(pinsStanding) + "  PINS UP", 784, 269, 28, cream);
        }
        // Dedicated video area. The feed draws only the deck, so it cannot sample this board.
        if (replayFeed) {
            DrawTexturePro(*replayFeed, {0, 0, float(replayFeed->width), -float(replayFeed->height)},
                           {265, 22, 494, 278}, {0, 0}, 0, WHITE);
            arcadeCenter("PIN DECK", 139, 136, 37, gold);
            arcadeCenter("REPLAY", 882, 136, 37, gold);
        }
        if (!replayFeed && cue == bowling::StadiumCue::Turkey) {
            sprite(sunburst, {29, 24, 966, 275});
            arcadeCenter("TURKEY!", 260, 116, 88, gold);
            arcadeCenter("RAYS UP!", 260, 212, 48, cream);
        }
        // These frame scores are live game data.
        for (int frame = 0; frame < 10; ++frame)
            arcadeCenter(std::to_string(frame + 1), 185 + frame * 68, 305, 20, gold);
        arcadeCenter("TOTAL", 908, 305, 20, gold);
        for (int player = 0; player < 2; ++player) {
            const auto &game = match.games[player];
            const Color rowInk = manual ? cream : (player ? homeInk : visitorInk);
            const float y = 350 + player * 62.f;
            arcadeCenter(player ? "HOME" : "VISITORS", 88, y, 26, rowInk);
            for (int frame = 0; frame < 10; ++frame) {
                const float cx = 185 + frame * 68;
                const auto &value = visibleTiles[player][frame];
                const auto &old = oldTiles[player][frame];
                if (manual && tileSeconds > 0 && old != value) {
                    const float phase = 1.f - tileSeconds / .42f;
                    const float height = 29.f * std::abs(std::cos(PI * phase));
                    if (height > 3)
                        arcadeCenter(phase < .5f ? old : value, cx, y + (29 - height) / 2, height, rowInk);
                } else
                    arcadeCenter(value, cx, y, 29, rowInk);
                if (!game.complete && frame == game.frame && player == match.player)
                    DrawLineEx({cx - 11, y + 31}, {cx + 11, y + 30}, 3, gold);
            }
            arcadeCenter(std::to_string(visibleTotals[player]), 908, y - 2, 33, rowInk);
        }
        const std::string welcome = cue == bowling::StadiumCue::TropicanaUnlocked
            ? "TROPICANA FIELD UNLOCKED"
            : mascot ? "RAYS UP!" : std::string("WELCOME TO ") + bowling::arenas[arena].name;
        arcadeCenter(welcome, 512, manual ? 472 : 451, manual ? 21 : 25, cream);
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
    void floatingMedal(Texture2D face, Vector3 pos, float size, float time) {
        rlPushMatrix();
        rlTranslatef(pos.x, pos.y, pos.z);
        rlRotatef(time * 75 + pos.z, 0, 1, 0);
        const float r = size * .5f;
        DrawCylinderEx({0,0,-.11f}, {0,0,.11f}, r, r, 32, {164,108,44,255});
        for (int i = 0; i < 28; ++i) {
            float a = 2*PI*i/28;
            DrawLine3D({r*std::cos(a),r*std::sin(a),-.10f},
                       {r*std::cos(a),r*std::sin(a),.10f}, gold);
        }
        for (int side = 0; side < 2; ++side) {
            sign(face, {0,0,-.115f}, size, size, true);
            rlRotatef(180, 0, 1, 0);
        }
        rlPopMatrix();
    }
    // The frame stays readable edge-on. Its enamel face rocks on the vertical axis.
    void rideEmblem(bowling::RidePower power, Vector3 pos, float time) {
        const bool waveFace = power == bowling::RidePower::Wave;
        const Color edge = waveFace ? Color{28, 113, 143, 255} : Color{107, 45, 136, 255};
        const Color bevel = waveFace ? Color{137, 250, 233, 255} : Color{255, 168, 236, 255};
        const int points = waveFace ? 10 : 4;
        const float radius = waveFace ? 1.33f : 1.47f;
        rlPushMatrix();
        rlTranslatef(pos.x, pos.y, pos.z);
        rlRotatef(44.f * std::sin(time * 2.0f + pos.z), 0, 1, 0);
        // A scalloped sea-star mount and a four-corner baseball diamond.
        for (int i = 0; i < points; ++i) {
            auto corner = [&](int index, float z, float scale) {
                float angle = PI / 2 + 2 * PI * index / points;
                float r = radius * (waveFace && index % 2 ? .76f : 1.f) * scale;
                return Vector3{r * std::cos(angle), r * std::sin(angle), z};
            };
            auto a = corner(i, 0, 1), b = corner(i + 1, 0, 1);
            for (int side : {-1, 1}) {
                auto c = corner(i, side * .17f, .88f), d = corner(i + 1, side * .17f, .88f);
                auto tri = [&](Vector3 x, Vector3 y, Vector3 z, Color face) {
                    if (side < 0) std::swap(y, z);
                    DrawTriangle3D(x, y, z, face);
                };
                tri(a, b, d, i % 3 == 0 ? cream : bevel);
                tri(a, d, c, bevel);
                tri(c, d, {0, 0, side * .17f}, edge);
            }
        }
        const float diameter = waveFace ? 1.81f : 1.84f;
        for (int face = 0; face < 2; ++face) {
            sign(rideBadges[int(power)], {0, 0, -.185f}, diameter, diameter, true);
            rlRotatef(180, 0, 1, 0);
        }
        rlPopMatrix();
        // A narrow, colored pool of light marks the pickup's lane position.
        for (int i = 0; i < 24; ++i) {
            const float a = 2 * PI * i / 24, b = a + 2 * PI / 24;
            DrawTriangle3D({pos.x, .062f, pos.z},
                           {pos.x + .79f * std::cos(b), .061f, pos.z + 1.03f * std::sin(b)},
                           {pos.x + .79f * std::cos(a), .061f, pos.z + 1.03f * std::sin(a)},
                           Fade(bevel, .52f));
        }
    }

    void drawSling(float x, float contact, float pull) {
        const Color leather{133, 48, 42, 255}, welt{242, 147, 75, 255};
        const float drawZ = 2.4f - contact * (1.22f + 2.7f * pull);
        const float saddleX = x * contact;
        for (int side : {-1, 1}) {
            const float anchorX = side * 4.8f;
            cyl({anchorX, .04f, 2.4f}, .24f, 1.31f, {21, 65, 82, 255});
            for (int wrap = 0; wrap < 4; ++wrap)
                cyl({anchorX, .36f + wrap * .18f, 2.4f}, .252f, .065f, {211, 175, 113, 255});
            cyl({anchorX, 1.03f, 2.4f}, .283f, .22f, {161, 96, 48, 255});
            orb({anchorX, 1.39f, 2.4f}, {.31f, .31f, .31f}, gold);
            Vector3 start{anchorX, 1.15f, 2.4f};
            Vector3 end{saddleX + side * .98f, 1.06f - contact * .49f, drawZ};
            Vector3 previous = start;
            for (int segment = 1; segment <= 8; ++segment) {
                const float t = segment / 8.f;
                Vector3 next = Vector3Lerp(start, end, t);
                next.y -= .16f * (1 - pull) * std::sin(t * PI);
                const float r = .086f - pull * .018f;
                beam(previous, next, r, leather);
                beam({previous.x, previous.y + r * .70f, previous.z - .022f},
                     {next.x, next.y + r * .70f, next.z - .022f}, .027f, welt);
                previous = next;
            }
            orb(end, {.105f, .12f, .115f}, gold);
            // The padded yoke wraps the near curve of the inflatable.
            previous = end;
            for (int i = 1; i <= 8; ++i) {
                const float t = i / 8.f;
                Vector3 next{saddleX + side * .98f * (1 - t), end.y - .032f * t,
                             drawZ - .61f * std::sin(t * PI / 2) * contact};
                beam(previous, next, .112f, {49, 75, 79, 255});
                beam({previous.x, previous.y + .080f, previous.z},
                     {next.x, next.y + .080f, next.z}, .036f, {220, 179, 111, 255});
                previous = next;
            }
        }
    }

    void rideWake(Vector3 p, const bowling::Body &ball, float elapsed) {
        const float length = std::min(3.7f, (p.z - geo::foulLine) * .68f);
        if (length < .12f) return;
        for (int side : {-1, 1}) {
            for (int j = 0; j < 12; ++j) {
                float a = j / 12.f, b = (j + 1) / 12.f;
                auto edge = [&](float t, float width) {
                    float spread = .92f + .31f*t + .07f*std::sin(9*t-elapsed*8)*t;
                    return Vector3{p.x + side*(spread + width) + ball.vx / std::max(1.f,ball.vz)*length*t,
                                   .071f + .025f*(1-t), p.z - .52f - t*length};
                };
                Vector3 strip[]{edge(a, 0), edge(a, .13f*(1-a)), edge(b, 0), edge(b, .13f*(1-b))};
                if (side < 0) { std::swap(strip[0],strip[1]); std::swap(strip[2],strip[3]); }
                DrawTriangleStrip3D(strip, 4, Fade({200, 251, 248, 255}, .68f*(1-a)));
            }
        }
    }

    void pickupFlash(Vector3 p, bowling::RidePower kind, float t) {
        const Color light = kind == bowling::RidePower::Wave ? Color{139, 255, 240, 255}
                                                              : Color{255, 172, 248, 255};
        for (int i = 0; i < 9; ++i) {
            const float angle = 2*PI*i/9 + t*.8f;
            const float r = 1.48f*(1-t) + .22f;
            Vector3 star{p.x + r*std::cos(angle), .71f + t*1.39f + .17f*std::sin(angle),
                         p.z + r*std::sin(angle)};
            const float size = .11f*std::sin(t*PI);
            orb(star, {size*.65f, size*1.6f, size*.65f}, light);
        }
    }

    void splash(Vector3 origin, float t, Camera3D camera) {
        const float radius = .38f + 3.95f * (1 - (1 - t) * (1 - t));
        const float swell = std::sin(PI * std::sqrt(t));
        rlPushMatrix();
        rlTranslatef(origin.x, origin.y, origin.z);
        // A broken crown with individual lips of foam. The pin silhouettes show through the gaps.
        for (int i = 0; i < 72; ++i) {
            const float a = 2 * PI * i / 72, b = 2 * PI * (i + 1) / 72;
            auto crest = [&](float angle) {
                float h = .14f + .70f * std::pow(.5f + .5f * std::sin(angle * 7 + .8f*std::sin(angle*3)), 2.f);
                float lip = radius + h * swell * .21f;
                return Vector3{lip * std::cos(angle), .02f + swell * h, lip * std::sin(angle)};
            };
            if (i % 17 < 2) continue;
            Vector3 c = crest(a), d = crest(b);
            Vector3 p{c.x * .84f, .012f, c.z * .84f}, q{d.x * .84f, .012f, d.z * .84f};
            const Color foot = Fade({41, 172, 204, 255}, .06f*(1-t));
            const Color lip = Fade({153, 237, 245, 255}, .68f*(1-t));
            auto vertex = [](Vector3 v, Color color) {
                rlColor4ub(color.r, color.g, color.b, color.a);
                rlVertex3f(v.x, v.y, v.z);
            };
            rlBegin(RL_QUADS);
            vertex(p, foot); vertex(c, lip); vertex(d, lip); vertex(q, foot);
            vertex(q, foot); vertex(d, lip); vertex(c, lip); vertex(p, foot);
            rlEnd();
            const float foam = .012f + .019f*swell;
            beam(c, d, foam, Fade({222, 255, 247, 255}, .84f*(1-t)));
        }
        for (int i = 0; i < 22; ++i) {
            const float a = i * 2.39996f;
            const float travel = radius * (.72f + .14f * (i % 4));
            const float h = swell * (.48f + .29f * (i % 6));
            float r = (.030f + .011f * (i % 3)) * (1 - t);
            orb({travel * std::cos(a), h, travel * std::sin(a)}, {r, r * 1.8f, r},
                i % 3 ? Color{133, 239, 246, 255} : WHITE);
        }
        // Put illustrated sheets of spray on the far and side lips. Their bases
        // travel out with the crown, then settle. The central lane stays open.
        Vector3 right = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camera.target, camera.position),
                                                            camera.up));
        rlSetTexture(spray.id);
        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, static_cast<unsigned char>(224.f * swell));
        for (float a : {.20f, 1.14f, 2.06f, 2.96f}) {
            float extent = 1.2f + 1.1f*swell;
            Vector3 base{radius*.78f*std::cos(a), .05f, radius*.78f*std::sin(a)};
            Vector3 left = Vector3Subtract(base, Vector3Scale(right, extent*.5f));
            Vector3 end = Vector3Add(base, Vector3Scale(right, extent*.5f));
            float height = extent*(.64f + .12f*std::sin(a*3));
            float u = std::cos(a)*right.x >= 0 ? 0.f : 1.f;
            rlTexCoord2f(u, 1); rlVertex3f(left.x, left.y, left.z);
            rlTexCoord2f(1-u, 1); rlVertex3f(end.x, end.y, end.z);
            rlTexCoord2f(1-u, 0); rlVertex3f(end.x, end.y+height, end.z);
            rlTexCoord2f(u, 0); rlVertex3f(left.x, left.y+height, left.z);
        }
        rlEnd();
        rlSetTexture(0);
        rlPopMatrix();
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
    void bowlingBall(Vector3 p, float roll, float radius, float sideRoll) {
        rlPushMatrix();
        rlTranslatef(p.x, p.y, p.z);
        rlRotatef(sideRoll, 0, 0, 1);
        rlRotatef(roll, 1, 0, 0);
        Texture2D original = sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture;
        sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skins[skin];
        material(7);
        orb({0, 0, 0}, {radius, radius, radius}, WHITE);
        material(0);
        sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = original;
        rlPopMatrix();
    }
    Vector2 pinLean(const bowling::Body &p) const {
        // World X runs opposite to simulation X. A wobbly corner circles its foot once.
        float angle = p.motion == bowling::PinMotion::Teeter ? p.motionTime * 4.f * PI : 0.f;
        return {-p.hitX * std::cos(angle) - p.hitZ * std::sin(angle),
                 p.hitZ * std::cos(angle) - p.hitX * std::sin(angle)};
    }
    void drawPin(const bowling::Body &p, geo::LaneLayout lane) {
        if (!p.down && setter.progress > 0) {
            float grip = bowling::PinsetterCycle::ease(setter.progress, .03f, .17f)
                       - bowling::PinsetterCycle::ease(setter.progress, .92f, 1.f);
            if (grip > .01f) {
                float y = geo::pinHeight + setter.lift() + (1-grip)*1.65f;
                Vector3 at{geo::renderX(p.x), y, p.z};
                beam(at, {at.x, y+.88f*grip, at.z}, .085f, {132, 147, 144, 255});
                DrawCylinderEx({at.x,y-.16f,at.z}, {at.x,y+.11f,at.z}, .27f*grip, .19f*grip,
                               14, {61, 73, 70, 255});
            }
        }
        rlPushMatrix();
        const float tilt = p.tiltDegrees(), cosine = std::cos(tilt * DEG2RAD);
        const float tip = std::abs(std::sin(tilt * DEG2RAD));
        const Vector2 lean = pinLean(p);
        // Support the belly and then the crown through an inverted shoulder tumble.
        const float lift = std::max({.30f * tip, geo::pinRadius * tip - .9f * cosine,
                                     -geo::pinHeight * cosine});
        float z = setter.pinZ(p, lane);
        float pitDrop = bowling::PinsetterCycle::ease(z, lane.surfaceEnd() - .4f, lane.surfaceEnd() + 1.f);
        rlTranslatef(geo::renderX(p.x), .06f + lift + (p.down ? -1.3f * pitDrop : setter.lift()), z);
        rlRotatef(tilt, lean.y, 0, -lean.x);
        rlScalef(geo::pinRadius / .40f, geo::pinHeight / 2.05f, geo::pinRadius / .40f);
        material(9);
        DrawModel(pin, {0, 0, 0}, 1, WHITE);
        material(0);
        rlPopMatrix();
    }
    void rider(Vector3 p, float time, bool ready, bowling::MascotPose pose, bool gutter) {
        const float wipeout = gutter ? pose.wipeout : 0.f;
        const float bob = std::sin(time * 3.f) * .028f;
        rlPushMatrix();
        rlTranslatef(p.x, p.y + bob - wipeout * .24f - pose.pull * .05f, p.z + pose.recoil * .18f);
        rlRotatef(bob * 42 - pose.bank * 20 + wipeout * (p.x < 0 ? -74.f : 74.f), 0, 0, 1);
        rlRotatef(pose.bank * 12 + wipeout * 55.f, 0, 1, 0);
        rlRotatef(-pose.pull * 3.f + pose.recoil * 10.f, 1, 0, 0);
        rlPushMatrix();
        rlScalef(1 + pose.pull * .035f, 1 - pose.pull * .13f, 1 + pose.pull * .025f);
        DrawModel(tube, {0, 0, 0}, 1, WHITE);
        rlPopMatrix();
        rlTranslatef(0, .12f - pose.pull * .10f, -.09f * pose.pull);
        float glance = bowling::PinsetterCycle::ease(pose.pull, .68f, 1.f);
        float turn = ready ? 180.f + std::sin(time * 1.5f) * 4.f + glance * 30.f : pose.bank * 10;
        turn += 155.f * bowling::PinsetterCycle::ease(strikeSalute, 0, .28f);
        rlRotatef(turn, 0, 1, 0);
        const float tip = std::sin(PI * bowling::PinsetterCycle::ease(strikeSalute, .16f, .89f));
        SetShaderValue(shader, GetShaderLocation(shader, "capTip"), &tip, SHADER_UNIFORM_FLOAT);
        rlRotatef(pose.pull * 7 - pose.recoil * 9, 1, 0, 0);
        const Vector3 finLoad{pose.pull, pose.bank, pose.recoil};
        SetShaderValue(shader, GetShaderLocation(shader, "finLoad"), &finLoad, SHADER_UNIFORM_VEC3);
        material(6);
        DrawModel(rayBody, {0, 0, 0}, 1, WHITE);
        material(0);
        const float level = 0;
        SetShaderValue(shader, GetShaderLocation(shader, "capTip"), &level, SHADER_UNIFORM_FLOAT);
        const Vector3 resting{};
        SetShaderValue(shader, GetShaderLocation(shader, "finLoad"), &resting, SHADER_UNIFORM_VEC3);
        rlPopMatrix();
    }

    void landmarks(float time) {
        Color brick = {145, 72, 48, 255};
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
            beam(last, next, radius, c);
            last = next;
        }
    }
    void mascotScenery(float time, Color team, int openSide) {
        const float extra = geo::mascotLane.sceneryExtension();
        Color rock = {45, 119, 165, 255};
        material(10);
        if (openSide == 0)
            DrawModel(course, {0, 0, 0}, 1, WHITE);
        material(3);
        box({0, 5.5f, 42.5f + extra}, {20, 11, 1}, rock);
        material(0);
        // The end of the ride opens onto a single large exhibit beyond the last pin.
        const float endWindow = 31.2f + extra;
        box({0, 4.92f, endWindow + .24f}, {15.3f, 10.f, .38f}, {21, 54, 63, 255});
        sign(reef, {0, 4.91f, endWindow}, 14.8f, 9.8f, true);
        for (int side : {-1, 1}) {
            beam({side * 7.43f, .08f, endWindow - .03f}, {side * 7.43f, 9.82f, endWindow - .03f},
                 .13f, {183, 142, 85, 255});
        }
        material(4);
        for (int side : {-1, 1})
            box({side * 6.7f, -.13f, 15 + extra / 2}, {2.45f, .10f, 46 + extra}, {27, 157, 193, 255});
        material(0);
        // Recessed exhibit windows follow the outer walls, leaving the course clear.
        for (int side : {-1, 1}) {
            if (side == openSide) continue;
            for (float z = 1; z < 34 + extra; z += 11.f) {
                rlPushMatrix();
                rlTranslatef(side * 7.05f, 4.2f, z);
                rlRotatef(side * 90.f, 0, 1, 0);
                for (int half : {-1, 1}) {
                    float worldZ = 2.f + (z-2.f - side*half*2.73f)*sceneryDepthScale;
                    auto exhibit = worldZ < geo::mascotLane.at(.32f) ? tank :
                                   worldZ < geo::mascotLane.at(.71f) ? kelp : reef;
                    sign(exhibit, {half*2.73f, 0, 0}, 5.45f, 7.6f, true);
                }
                beam({0, -3.82f, -.025f}, {0, 3.82f, -.025f}, .067f, {58, 107, 111, 255});
                for (float y : {-3.85f, 3.85f})
                    beam({-5.5f, y, 0}, {5.5f, y, 0}, .12f, {73, 157, 161, 255});
                rlPopMatrix();
            }
        }
        // Layers of close scenery make the ride feel embedded in a themed course.
        for (int side : {-1, 1})
            for (int i = 0; i < 9 + int(extra / 4.7f) + 1; ++i) {
                if (side == openSide) break;
                float z = -3 + i * 4.7f;
                material(3);
                orb({side * (8.4f + .3f * std::sin(i * 3.f)), 1.2f, z},
                    {1.1f, 1.8f + .8f * std::sin(i * 2.f), 2.4f}, rock);
                material(0);
                cyl({side * 6.35f, .05f, z}, .18f, 1.1f, {22, 53, 86, 255});
                orb({side * 6.35f, 1.2f, z}, {.22f, .25f, .22f}, {135, 246, 255, 255});
            }
        for (float z : {4.f, 16.f + extra / 2, 29.f + extra}) {
            const Color bronze{151, 107, 57, 255}, trim{213, 171, 102, 255};
            arch(z, 6.83f, 8.65f, {31, 71, 78, 255}, .27f);
            arch(z - .15f, 6.80f, 8.60f, bronze, .10f);
            arch(z - .19f, 6.64f, 8.41f, {121, 226, 218, 255}, .049f);
            for (int side : {-1, 1}) {
                box({side * 6.8f, .55f, z}, {.64f, 1.10f, .70f}, team);
                box({side * 6.8f, 1.1f, z}, {.74f, .13f, .83f}, trim);
                for (float a : {.15f, .44f, .78f, 1.13f}) {
                    Vector3 joint{side * 6.83f * std::cos(a), .35f + 8.65f * std::sin(a), z - .18f};
                    orb(joint, {.16f, .16f, .09f}, bronze);
                    joint.z -= .07f;
                    orb(joint, {.072f, .072f, .04f}, trim);
                }
            }
        }
        // Foreground ribbons break the straight window sill. Different clusters
        // pass at different depths, giving the exhibit parallax during the ride.
        for (int side : {-1, 1}) {
            if (side == openSide) continue;
            for (int bed = 0; bed < 9; ++bed) {
                const float z = -2 + bed * 10.5f + (side < 0 ? 3.4f : 0.f);
                for (int j = 0; j < 7; ++j) {
                    const float seed = float(bed * 13 + j * 7 + side);
                    const float x = side * (6.53f + .39f * std::sin(seed * 1.26f));
                    const float depth = z + std::sin(seed * .75f) * 1.47f;
                    float worldZ = 2.f + (z-2.f)*sceneryDepthScale;
                    bool forest = worldZ >= geo::mascotLane.at(.32f) && worldZ <= geo::mascotLane.at(.71f);
                    const float height = (.8f + .92f*(1+std::sin(seed*.82f))) * (forest ? 2.6f : 1.f);
                    Vector3 leaf[12];
                    for (int joint = 0; joint < 6; ++joint) {
                        const float t = joint / 5.f;
                        const float drift = (.34f * std::sin(3.6f*t + seed + time*.72f)
                                             + .27f * std::cos(seed)) * t;
                        const float width = (.035f + (forest ? .37f : .14f) * std::sin(t*PI)) * (1 - t);
                        Vector3 knot{x + drift, .13f + height*t, depth + .19f*t*t};
                        leaf[joint*2] = {knot.x - width, knot.y, knot.z};
                        leaf[joint*2+1] = {knot.x + width, knot.y, knot.z + width*.45f};
                    }
                    DrawTriangleStrip3D(leaf, 12, j % 2 ? Color{68, 147, 114, 255} : Color{25, 96, 88, 255});
                    // The curving blades can be seen from either end of the exhibit.
                    for (int joint = 0; joint < 6; ++joint)
                        std::swap(leaf[joint*2], leaf[joint*2+1]);
                    DrawTriangleStrip3D(leaf, 12, {34, 106, 92, 255});
                }
            }
        }
        for (int side : {-1, 1}) {
            for (int j = 0; j < 5; ++j)
                orb({side * (5.4f + j * .18f), 2.2f + j * .5f, 30.4f + extra}, {.23f, .23f, .23f}, gold);
            sign(scoreboard.texture, {side * 7.0f, 4.1f, 14.5f + extra / 2}, 3.1f, 1.55f);
        }
        // Tropicana's aquarium theme: suspended rays and bubbles beyond the rails.
        if (mascot)
            for (int side : {-1, 1}) {
                if (side == openSide) continue;
                for (int i = 0; i < 3; ++i) {
                    float z = 8 + i * (9.f + extra / 3);
                    const float swimPhase = time * .75f + i * 1.7f + side;
                    const float swimZ = z + std::sin(swimPhase) * 3.4f;
                    const float swimX = side * (6.48f + .25f * std::cos(swimPhase));
                    const float swimY = 4.4f + .35f * std::sin(swimPhase * 2);
                    rlPushMatrix();
                    rlTranslatef(swimX, swimY, swimZ);
                    rlRotatef(std::cos(swimPhase) * 75.f, 0, 1, 0);
                    rlScalef(.68f, .55f, .55f);
                    material(6);
                    DrawModel(rayBody, {0, 0, 0}, 1, WHITE);
                    material(0);
                    rlPopMatrix();
                    for (int j = 0; j < 5; ++j) {
                        float y = std::fmod(time * .55f + j * 1.13f + i, 5.f) + 1;
                        orb({side * (6.9f + .2f * std::sin(j * 3.f)), y, z}, {.045f, .065f, .045f},
                            {169, 235, 246, 255});
                    }
                }
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
    void ballReturn(Color paint) {
        // Twin rubber rails lead out of an open, enamelled sheet-metal hood.
        constexpr float x = -7.65f, mouth = 9.3f;
        Color chrome{164, 174, 171, 255}, rubber{36, 40, 44, 255};
        box({x, .18f, 7.5f}, {2.57f, .36f, 6.85f}, paint);
        for (int side : {-1, 1}) {
            for (float z : {4.5f, 7.2f, 10.3f})
                cyl({x + side*.55f, .32f, z}, .10f, .46f, chrome);
            beam({x + side*.55f, .79f, 4.29f}, {x + side*.55f, .86f, 10.45f}, .125f, rubber);
        }
        beam({x-.93f,.94f,4.23f}, {x+.93f,.94f,4.23f}, .17f, rubber);
        box({x, 1.15f, 11.3f}, {2.3f, 1.85f, .22f}, rubber);
        constexpr int segments = 18;
        auto rim = [&](int i, float z) {
            float a = PI*i/segments;
            return Vector3{x+1.25f*std::cos(a), .42f+2.08f*std::sin(a), z};
        };
        for (int i=0; i<segments; ++i) {
            auto a=rim(i,mouth), b=rim(i+1,mouth);
            auto c=rim(i,11.5f), d=rim(i+1,11.5f);
            Color enamel=ColorBrightness(paint, .24f*std::sin(PI*i/segments));
            DrawTriangle3D(a,b,c,enamel); DrawTriangle3D(b,d,c,enamel);
            DrawTriangle3D(a,c,b,rubber); DrawTriangle3D(b,c,d,rubber);
            beam(a,b,.061f,chrome);
        }
        if (returningBall) {
            const float t = bowling::PinsetterCycle::ease(setter.progress, .63f, .96f);
            float z = 10.6f - 5.45f*t;
            const float radius = geo::ballRadius;
            bowlingBall({x, 1.57f-.07f*t, z}, (z-5.15f)*RAD2DEG/radius, radius, 0);
        }
    }

    void pinsetter(geo::LaneLayout lane, Color paint) {
        const float end = lane.surfaceEnd(), z = setter.bladeZ(lane);
        const float y = 4.32f - 3.83f * setter.lowered();
        const Color metal = mascot ? Color{183, 141, 82, 255} : Color{157, 173, 174, 255};
        const Color frame = mascot ? Color{26, 82, 86, 255} : Color{49, 64, 68, 255};
        const Color rubber{31, 34, 36, 255};
        // Behind the playable maple is a shallow conveyor and a row of rubber impact flaps.
        box({0, -.91f, end + 1.55f}, {10.9f, .28f, 3.25f}, rubber);
        for (int i = -6; i <= 6; ++i) {
            float x = i * .82f;
            box({x, .48f, end + 2.78f}, {.79f, 1.83f, .22f}, i % 2 ? rubber : Color{40, 44, 45, 255});
            beam({x - .21f, 1.28f, end + 2.64f}, {x + .21f, 1.28f, end + 2.64f}, .038f, metal);
        }
        for (int side : {-1, 1}) {
            const float x = side * 5.48f;
            box({x, -.23f, end + 1.45f}, {.41f, 1.24f, 3.4f}, frame);
            box({x, 2.35f, end + 1.78f}, {.32f, 4.9f, .47f}, frame);
            box({x, 4.67f, (lane.headPin()+end)*.5f}, {.28f, .25f, 17.f}, metal);
            Vector3 trolley{x, 4.48f, z}, elbow{x, (y + 4.53f)*.5f, z + .55f};
            beam(trolley, elbow, .095f, metal);
            beam(elbow, {x, y, z}, .092f, frame);
            orb(elbow, {.15f, .15f, .15f}, metal);
            box(trolley, {.51f, .26f, .62f}, frame);
            if (mascot) {
                // Slender glazing with a bright top edge beside the brass pit frame.
                DrawCubeV({x, .48f, end + 1.55f}, {.069f, .67f, 3.12f}, {126, 224, 218, 122});
                box({x, .83f, end + 1.55f}, {.16f, .05f, 3.14f}, metal);
            }
        }
        box({0, 1.49f, end+2.8f}, {11.4f, .22f, .4f}, frame);
        box({0, y, z}, {11.13f, .52f, .35f}, metal);
        box({0, y, z-.19f}, {10.88f, .35f, .05f}, mascot ? frame : paint);
        box({0, y-.27f, z}, {10.9f, .15f, .37f}, rubber);
        for (int side : {-1, 1}) {
            for (int stripe = 0; stripe < 3; ++stripe) {
                float x = side*(3.2f + stripe*.46f);
                beam({x, y-.11f, z-.24f}, {x+side*.22f, y+.11f, z-.24f}, .052f, cream);
            }
            orb({side*5.18f, y, z-.23f}, {.087f, .087f, .034f}, metal);
        }
        orb({0, y, z-.22f}, {.36f, .36f, .07f}, metal);
        sign(teamLogos[mascot ? 9 : arena], {0, y, z-.30f}, .55f, .55f, true);
    }

    void fieldBox(float x, float z, float time, Color team) {
        const Color rail{77, 99, 91, 255};
        box({x, .37f, z}, {8.3f, .85f, 1.6f}, rail);
        for (int i = 0; i < 7; ++i) {
            float sx = x + (i-3)*1.07f;
            float rise = crowdCheer * (.8f + .2f * std::sin(time*6.f + i));
            float y = 1.37f + rise*.72f + (i%3)*.066f;
            Color jersey = i%3 == 0 ? cream : (i%3 == 1 ? team : Color{156, 67, 50, 255});
            Color skinTone = i%2 ? Color{173, 116, 76, 255} : Color{220, 166, 125, 255};
            orb({sx, y, z}, {.29f, .44f, .20f}, jersey);
            orb({sx, y+.52f, z}, {.195f,.25f,.19f}, skinTone);
            orb({sx, y+.68f, z}, {.21f,.14f,.20f}, team);
            box({sx, y+.64f, z-.17f}, {.36f,.045f,.24f}, team);
            for (int hand : {-1,1}) {
                float sway = std::sin(time*8.f+i+hand)*rise*.18f;
                Vector3 shoulder{sx+hand*.21f, y+.22f, z};
                Vector3 elbow{sx+hand*(.33f+.09f*rise), y+.02f+rise*.49f, z-.10f};
                Vector3 palm{elbow.x+sway, y+.02f+rise*.87f, z-.25f};
                beam(shoulder, elbow, .10f, jersey);
                beam(elbow, palm, .066f, skinTone);
                if (hand == 1 && i % 3 == 1) {
                    Vector3 tip{palm.x + sway, palm.y + .67f, palm.z};
                    beam(palm, tip, .024f, cream);
                    Vector3 fly{tip.x-.54f, tip.y-.17f+.08f*rise*std::sin(time*10+i), tip.z};
                    DrawTriangle3D(tip, fly, {tip.x, tip.y-.31f, tip.z}, team);
                    DrawTriangle3D(tip, {tip.x, tip.y-.31f, tip.z}, fly, team);
                }
            }
        }
        box({x, 1.21f, z-.47f}, {8.45f,.13f,.14f}, rail);
    }

    void stadiumScenery(float time, Color team) {
        const float extra = geo::sceneryExtension;
        material(8);
        for (int stripe = 0; stripe < 18 + int(extra / 4) + 2; ++stripe) {
            float z = -12.f + stripe * 4;
            Color grass = stripe % 2 ? Color{47, 111, 57, 255} : Color{58, 129, 65, 255};
            float worldZ = 2.f + (z-2.f)*sceneryDepthScale;
            if (std::abs(worldZ - (geo::stadiumLane.surfaceEnd()+1.5f)) < 6.f) {
                for (int side : {-1, 1})
                    box({side*32.19f, -.24f, z}, {53.62f, .12f, 4.01f}, grass);
            } else box({0, -.24f, z}, {118, .12f, 4.01f}, grass);
        }
        material(0);
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
        // Continuous outfield panorama curves into the sidelines behind the live board.
        rlSetTexture(outfield[arena].id);
        rlBegin(RL_QUADS);
        rlColor4ub(255, 255, 255, 255);
        constexpr int sections = 56;
        for (int section = 0; section < sections; ++section) {
            for (int corner : {0, 1, 2, 3}) {
                float u = (section + (corner >= 2 ? 1.f : 0.f)) / sections;
                float angle = (u - .5f) * PI * 1.45f;
                bool top = corner == 1 || corner == 2;
                rlTexCoord2f(1.f-u, top ? 0.f : 1.f);
                rlVertex3f(118.f * std::sin(angle), top ? 109.f : -18.f,
                           extra * .5f + 4.f + (extra * .5f + 48.f) * std::cos(angle));
            }
        }
        rlEnd();
        rlSetTexture(0);
        for (int side : {-1, 1})
            for (float z : {7.f, 37.f + extra}) {
                cyl({side * 36.f, 0, z}, .18f, 22, {113, 133, 144, 255});
                box({side * 36.f, 22, z}, {5.4f, 2.5f, .4f}, {32, 49, 60, 255});
                for (int row = 0; row < 2; ++row)
                    for (int col = 0; col < 5; ++col)
                        orb({side * 36.f - 2 + col, 21.5f + row, z - .3f}, {.28f, .28f, .08f},
                            {255, 235, 185, 255});
            }
        for (int side : {-1, 1})
            fieldBox(side * 11.7f, 34 + extra, time, team);
        if (arena == 0) {
            // Exposed brick at the base of the ivy; the painted outfield wraps behind it.
            material(2);
            box({0, .66f, 43.8f+extra}, {103, 1.48f, .85f}, {145, 79, 55, 255});
            material(0);
            box({0, 1.40f, 43.8f+extra}, {103, .12f, .98f}, {176, 124, 84, 255});
            for (int bay = 0; bay < 3; ++bay) {
                float x = -23.5f + bay * 5.7f;
                beam({x, 1.4f, 42+extra}, {x+5.7f, 7.2f, 42+extra}, .16f, {87, 104, 93, 255});
                beam({x+5.7f, 1.4f, 42+extra}, {x, 7.2f, 42+extra}, .16f, {87, 104, 93, 255});
            }
        }
        // Stadium identity belongs on the outfield infrastructure, not over the pins.
        Color boardPaint = (arena == 0 || arena == 1) ? Color{58, 77, 51, 255} : Color{19, 23, 25, 255};
        float boardX = arena == 9 ? 0.f : -15.f, boardW = arena == 9 ? 39.f : (arena == 0 ? 19.f : 23.f);
        float boardY = arena == 9 ? 16.f : (arena == 0 ? 12.f : 14.f),
              boardH = boardW * .5f;
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

            // Tropicana's roof catwalk, orange landmark and flanking display pylons.
            for (float y : {27.f, 28.f})
                beam({-43, y, 40 + extra}, {43, y, 40 + extra}, .12f, {48, 59, 67, 255});
            for (int x = -42; x <= 42; x += 3)
                beam({float(x), 27, 40 + extra}, {float(x), 28, 40 + extra}, .07f, cream);
            material(11);
            orb({28, 17, 41 + extra}, {3.2f, 3.2f, 1.5f}, {255, 170, 16, 255});
            material(0);
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
        struct ParkLight { Vector3 direction, tint; float overhead; };
        static constexpr ParkLight lighting[] = {
            {{-.91f,.44f,-.34f}, {1.20f,.84f,.60f}, .10f}, // Wrigley sunset
            {{.69f,.61f,-.52f}, {1.10f,.91f,.68f}, .16f},  // Fenway copper afternoon
            {{-.46f,.91f,-.25f}, {.91f,1.02f,1.12f}, .08f}, // McCovey Cove
            {{.72f,.49f,-.55f}, {1.10f,.81f,.95f}, .14f},  // Pittsburgh river sunset
            {{-.65f,.83f,.13f}, {1.06f,.99f,1.12f}, .08f},  // Denver clear sky
            {{.34f,.78f,-.66f}, {.85f,.96f,1.18f}, .37f},   // Kauffman twilight
            {{-.83f,.64f,-.14f}, {1.20f,.89f,.67f}, .65f},  // Chase roof windows
            {{-.38f,.98f,.22f}, {.87f,1.05f,1.04f}, .46f},   // Seattle cloud cover
            {{.63f,.76f,-.52f}, {1.08f,1.00f,.88f}, .73f},   // Bronx floodlights
            {{.14f,.99f,-.24f}, {.79f,.98f,1.21f}, 1.f}    // Tropicana catwalks
        };
        const ParkLight aquarium{{-.35f,.85f,-.4f}, {1.f,.96f,.87f}, 0};
        const auto &light = mascot ? aquarium : lighting[arena];
        SetShaderValue(shader, GetShaderLocation(shader, "keyDirection"), &light.direction, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "keyTint"), &light.tint, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "roofLight"), &light.overhead, SHADER_UNIFORM_FLOAT);
        float run = sim.layout.run;
        SetShaderValue(shader, GetShaderLocation(shader, "laneRun"), &run, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "eye"), &cam.position, SHADER_UNIFORM_VEC3);
        Vector4 shadow[10];
        for (auto &entry : shadow) entry = {1000, 1000, 0, 0};
        for (size_t i = 0; i < sim.pins.size(); ++i) {
            const auto &p = sim.pins[i];
            const Vector2 direction = pinLean(p);
            const float length = std::sin(p.tiltDegrees() * DEG2RAD) * 2.28f;
            shadow[i] = {geo::renderX(p.x), setter.pinZ(p, sim.layout),
                         direction.x * length, direction.y * length};
        }
        SetShaderValueV(shader, GetShaderLocation(shader, "pinShadows"), shadow, SHADER_UNIFORM_VEC4, 10);
        Vector3 ballPos{geo::renderX(sim.hasBall ? sim.ball.x : shot.position), 0,
                        sim.hasBall ? sim.ball.z : geo::foulLine};
        const float slingDraw = mascot && aim ? mascotPose.pull * 2.7f : 0.f;
        if (sim.hasBall && setter.progress > 0) {
            const float coast = bowling::PinsetterCycle::ease(setter.progress, 0, .42f);
            if (!mascot) ballPos.z += coast * (sim.layout.surfaceEnd() + 1.4f - ballPos.z);
            else ballPos.x += coast * ((ballPos.x < 0 ? -4.25f : 4.25f) - ballPos.x);
        }
        Vector3 shadowBall{ballPos.x, sim.hasBall ? sim.ball.r : geo::ballRadius, ballPos.z - slingDraw};
        SetShaderValue(shader, GetShaderLocation(shader, "ballShadow"), &shadowBall,
                       SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "sceneTime"), &time, SHADER_UNIFORM_FLOAT);
        const float glow = cue == bowling::StadiumCue::Turkey
                               ? std::min(1.f, cueSeconds) * (.65f + .35f * std::sin(time * 6))
                               : 0.f;
        const Vector2 boardLight{glow, sim.layout.headPin()};
        SetShaderValue(shader, GetShaderLocation(shader, "boardLight"), &boardLight, SHADER_UNIFORM_VEC2);
        // The replay rig looks in from outside the left exhibit wall. Open that
        // side like a stage cutaway and carry the surrounding water to the horizon.
        const int openSide = mascot && std::abs(cam.position.x) > 7.05f ? (cam.position.x < 0 ? -1 : 1) : 0;
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
                mascotScenery(time, {16, 52, 94, 255}, openSide);
            rlPopMatrix();
            inSceneryTransform = false;
        }
        material(mascot ? 5 : 1);
        const float surfaceEnd = sim.layout.surfaceEnd();
        const float surfaceLength = surfaceEnd - geo::surfaceStart;
        const float surfaceCenter = (geo::surfaceStart + surfaceEnd) / 2;
        box({0, -.06f, surfaceCenter}, {geo::width, .2f, surfaceLength},
            mascot ? Color{158, 213, 224, 255} : Color{222, 171, 103, 255});
        material(0);
        for (float x : {-geo::gutterCenter, geo::gutterCenter}) {
            box({x, -.08f, surfaceCenter}, {geo::gutterWidth, .15f, surfaceLength},
                {16, 37, 55, 255});
            for (float d : {-.30f, .30f})
                beam({x + d, .10f, geo::surfaceStart}, {x + d, .10f, surfaceEnd}, .07f,
                     {107, 156, 176, 255});
            box({x + (x < 0 ? -.5f : .5f), .12f, surfaceCenter}, {.25f, .42f, surfaceLength},
                team);
            box({x + (x < 0 ? -.5f : .5f), .36f, surfaceCenter}, {.15f, .025f, surfaceLength},
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
            if (powerPickup && !powerPickup->claimed) {
                const auto &item = *powerPickup;
                const float x = geo::renderX(item.position.x);
                const float y = 1.04f + .13f * std::sin(time * 3.2f + item.position.z);
                floatingMedal(badges[int(item.power) - 1], {x, y, item.position.z}, 1.35f, time);
            }
            if (ridePickups && ridePickups->route)
                for (const auto &item : *ridePickups->route) {
                    if (item.claimed) {
                        const float t = (sim.elapsed - item.claimedAt) / .48f;
                        if (sim.active && t >= 0 && t < 1)
                            pickupFlash(ballPos, item.power, t);
                        continue;
                    }
                    const auto pos = item.position;
                    rideEmblem(item.power, {geo::renderX(pos.x), 1.58f + .13f * std::sin(time * 3 + pos.z),
                                            pos.z}, time);
                }
            if (mascotTokens)
                for (size_t i = 0; i < mascotTokens->layout.size(); ++i) {
                    if (mascotTokens->collected[i])
                        continue;
                    const auto token = mascotTokens->layout[i];
                    floatingMedal(tokenMedal,
                                  {geo::renderX(token.x), .95f + .14f * std::sin(time * 4 + i), token.z},
                                  1.5f, time);
                }
        }
        if (!mascot && !deckOnly) ballReturn(team);
        pinsetter(sim.layout, team);
        for (const auto &p : sim.pins)
            drawPin(p, sim.layout);
        if (mascot && sim.active && ballPos.z < sim.layout.headPin() && !sim.gutter)
            rideWake(ballPos, sim.ball, sim.elapsed);
        if (ballPos.z < surfaceEnd + .95f) {
            if (mascot)
                rider({ballPos.x, .57f,
                       ballPos.z - (aim ? mascotPose.pull * 2.7f : 0)},
                      time, aim, mascotPose, sim.gutter);
            else
                bowlingBall({ballPos.x, (sim.gutter ? .28f : shadowBall.y) -
                                 1.45f * bowling::PinsetterCycle::ease(ballPos.z, surfaceEnd - .3f, surfaceEnd + 1.f),
                             ballPos.z},
                            sim.hasBall ? (ballPos.z - geo::foulLine) * RAD2DEG / sim.ball.r : 0.f,
                            shadowBall.y, sim.hasBall ? (sim.ball.x - shot.position) * RAD2DEG / sim.ball.r : 0.f);
        }
        if (mascot && !deckOnly && (aim || (sim.active && sim.elapsed < .48f))) {
            const float spring = aim ? 0 : std::exp(-10 * sim.elapsed) * std::cos(23 * sim.elapsed);
            drawSling(aim ? ballPos.x : geo::renderX(shot.position),
                      aim ? 1.f : spring, aim ? mascotPose.pull : shot.power);
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
        if (mascot && wave) {
            const float progress = wave->progress(sim.elapsed);
            if (progress >= 0 && progress < 1) {
                rlDrawRenderBatchActive();
                rlDisableDepthMask();
                splash({geo::renderX(wave->center.x), .08f, wave->center.z}, progress, cam);
                rlDrawRenderBatchActive();
                rlEnableDepthMask();
            }
        }
        EndMode3D();
    }
    void close() {
        for (auto texture : reactions)
            UnloadTexture(texture);
        for (auto mural : outfield) UnloadTexture(mural);
        for (auto texture : {shop, strikePennant, gearCase, tank, kelp, reef, spray, sunburst, tokenMedal, manualBoard, electricBoard,
                             scorePlates, enamelLettering, pinGauge, scoreRails[0], scoreRails[1]})
            UnloadTexture(texture);
        for (auto texture : badges)
            UnloadTexture(texture);
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
        for (auto texture : rideBadges)
            UnloadTexture(texture);
        UnloadRenderTexture(scoreboard);
        UnloadShader(shader);
    }
};
