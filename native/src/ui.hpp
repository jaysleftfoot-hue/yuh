#pragma once
#include "raylib.h"
#include <algorithm>
#include <string>
inline Font uiFont{}, arcadeFont{};
inline bool uiBlocked = false;
inline const Color ink = {13, 24, 45, 255}, cream = {255, 241, 204, 255}, gold = {255, 194, 64, 255},
                   red = {216, 53, 57, 255}, aqua = {69, 223, 206, 255};
inline void text(const std::string &s, float x, float y, float size, Color c) {
    DrawTextEx(uiFont, s.c_str(), {x, y}, size, 1, c);
}
inline float tw(const std::string &s, float size) {
    return MeasureTextEx(uiFont, s.c_str(), size, 1).x;
}
inline void center(const std::string &s, float x, float y, float size, Color c) {
    text(s, x - tw(s, size) / 2, y, size, c);
}
inline void title(const std::string &s, float x, float y, float size, Color c) {
    for (int dx = -2; dx <= 2; dx += 2)
        for (int dy = -2; dy <= 2; dy += 2)
            DrawTextEx(arcadeFont, s.c_str(), {x + dx + 2, y + dy + 3}, size, 1, {15, 21, 20, 230});
    DrawTextEx(arcadeFont, s.c_str(), {x, y}, size, 1, c);
}
// Irregular stitched leather, never a rounded application card.
inline void panel(Rectangle r, Color c) {
    Vector2 middle{r.x + r.width / 2, r.y + r.height / 2};
    const int edges = 24;
    Vector2 points[edges];
    for (int i = 0; i < edges; ++i) {
        float a = i * 2 * PI / edges;
        float sx = std::cos(a), sy = std::sin(a);
        float scale = 1 / std::max(std::abs(sx), std::abs(sy));
        float wear = 1 - .025f * (i % 3);
        points[i] = {middle.x + sx * scale * r.width * .5f * wear,
                     middle.y + sy * scale * r.height * .5f * wear};
    }
    for (int i = 0; i < edges; ++i) {
        auto a = points[i], b = points[(i + 1) % edges];
        DrawTriangle(middle, b, a, c);
        DrawLineEx(a, b, 3, {116, 106, 75, 255});
        Vector2 aa{middle.x + (a.x - middle.x) * .96f, middle.y + (a.y - middle.y) * .88f};
        Vector2 bb{middle.x + (b.x - middle.x) * .96f, middle.y + (b.y - middle.y) * .88f};
        DrawLineEx(aa, bb, 1, {183, 164, 119, 170});
    }
    for (int i = 0; i < 55; ++i) {
        float x = r.x + 10 + std::fmod(i * 73.f, std::max(1.f, r.width - 20)),
              y = r.y + 8 + std::fmod(i * 31.f, std::max(1.f, r.height - 16));
        DrawLine(x, y, x + 3, y + 1, {0, 0, 0, 30});
    }
}
inline void baseballHud(Texture2D portraits, int rival) {
    // Interlocking leather pads and crossed bats frame the scene below the scores.
    for (int i = 0; i < 26; ++i) {
        float x = i * 50.f;
        DrawEllipse(x + 25, 87 + (i % 3) * 4, 36, 24, {43, 29, 18, 255});
        DrawEllipse(x + 24, 82 + (i % 3) * 4, 32, 20, {(unsigned char)(111 + i % 3 * 12), 72, 37, 255});
        DrawLineEx({x + 2, 75}, {x + 35, 94}, 2, {172, 127, 70, 255});
    }
    DrawLineEx({558, 128}, {720, 65}, 12, {53, 33, 19, 255});
    DrawLineEx({560, 125}, {718, 62}, 7, {213, 170, 91, 255});
    DrawLineEx({561, 63}, {718, 128}, 12, {53, 33, 19, 255});
    DrawLineEx({563, 60}, {716, 125}, 7, {213, 170, 91, 255});
    DrawTriangle({640, 160}, {699, 83}, {581, 83}, {36, 52, 48, 255});
    DrawTriangleLines({640, 160}, {699, 83}, {581, 83}, gold);
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col <= row; ++col)
            DrawCircle(640 + (col - row * .5f) * 19, 139 - row * 15, 5, {150, 242, 77, 255});
    panel({8, 130, 145, 566}, {49, 64, 70, 255});
    title("POWER UPS", 14, 274, 30, gold);
    // Vector-style illustrated busts replace the primitive oval/rectangle faces.
    const char *names[] = {"YOU", "SLUGGER", "UMPIRE", "BALL BOY"};
    for (int side = 0; side < 2; ++side) {
        float x = side ? 1206 : 80;
        int who = side ? rival : 0;
        panel({x - 68, 128, 136, 138}, {54, 70, 76, 255});
        float w = portraits.width / 2.f, h = portraits.height / 2.f;
        DrawTexturePro(portraits, {(who % 2) * w, (who / 2) * h, w, h}, {x - 63, 130, 126, 119}, {0, 0},
                       0, WHITE);
        title(names[who], x - 42, 244, 20, cream);
    }
}
inline void arcadeMeter(float x, float y, float power, Color c) {
    panel({x - 24, y - 140, 48, 174}, {57, 58, 48, 255});
    DrawRectangle(x - 14, y - 128, 28, 145, {12, 17, 19, 255});
    for (int i = 0; i < 12; ++i) {
        Color light = (i + 1) / 12.f <= power ? (i > 9 ? red : c) : Color{37, 43, 40, 255};
        DrawRectangleGradientV(x - 12, y + 3 - i * 11, 24, 9, ColorBrightness(light, .3f), light);
    }
    DrawTriangle({x, y - 153}, {x - 16, y - 136}, {x + 16, y - 136}, c);
}
inline bool button(Rectangle r, const std::string &s, bool primary = false, bool enabled = true) {
    enabled = enabled && !uiBlocked;
    bool hover = enabled && CheckCollisionPointRec(GetMousePosition(), r);
    float size = std::min(32.f, (r.width - 12) /
                                    std::max(1.f, MeasureTextEx(arcadeFont, s.c_str(), 32, 1).x) * 32);
    float width = MeasureTextEx(arcadeFont, s.c_str(), size, 1).x;
    title(s, r.x + (r.width - width) / 2, r.y + (r.height - size) / 2, size,
          enabled ? (hover ? WHITE : (primary ? gold : Color{150, 242, 77, 255})) : GRAY);
    if (hover)
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}
inline bool powerButton(int index, int stock, bool selected, bool enabled) {
    float x = 79, y = 350 + index * 104.f;
    bool hover = enabled && !uiBlocked && CheckCollisionPointCircle(GetMousePosition(), {x, y}, 40);
    DrawCircle(x + 3, y + 5, 39, {0, 0, 0, 150});
    DrawCircleGradient(x, y, 39, selected ? gold : Color{167, 179, 180, 255}, {37, 48, 55, 255});
    DrawCircleGradient(x, y, 31, hover ? Color{80, 161, 176, 255} : Color{34, 74, 89, 255},
                       {8, 23, 36, 255});
    Color c = enabled ? cream : GRAY;
    if (index == 0) {
        DrawCircleGradient(x, y, 20, c, {104, 99, 82, 255});
        for (int i = 0; i < 3; ++i)
            DrawCircle(x - 7 + i * 6, y - 7 + (i % 2) * 5, 3, ink);
    } else if (index == 1) {
        DrawRing({x, y - 4}, 12, 20, 0, 180, 24, c);
        DrawRectangle(x - 20, y - 19, 8, 17, red);
        DrawRectangle(x + 12, y - 19, 8, 17, aqua);
    } else {
        DrawLineEx({x - 17, y - 19}, {x - 17, y + 19}, 7, c);
        DrawLineEx({x + 17, y - 19}, {x + 17, y + 19}, 7, c);
        DrawCircle(x, y, 9, gold);
    }
    const char *names[] = {"HEAVY", "MAGNET", "BUMPERS"};
    center(std::string(names[index]) + "  " + std::to_string(stock), x, y + 43, 13, c);
    center(std::to_string(index + 1), x - 34, y - 39, 14, gold);
    return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}
