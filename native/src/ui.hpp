#pragma once
#include "raylib.h"
#include <algorithm>
#include <array>
#include <string>
inline Font uiFont{}, arcadeFont{};
inline Texture2D uiSlate{}, uiFrame{}, uiButton{};
inline Shader uiBallShader{};
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
inline void arcadeCenter(const std::string &s, float x, float y, float size, Color c = cream) {
    title(s, x - MeasureTextEx(arcadeFont, s.c_str(), size, 1).x / 2, y, size, c);
}
inline void sprite(Texture2D t, Rectangle r, Color tint = WHITE) {
    DrawTexturePro(t, {0, 0, float(t.width), float(t.height)}, r, {0, 0}, 0, tint);
}
inline void ballIcon(Texture2D skin, float x, float y, float size, bool unlocked = true) {
    DrawEllipse(x + size / 2 + 2, y + size * .91f, size * .46f, size * .13f, {0, 0, 0, 140});
    BeginShaderMode(uiBallShader);
    sprite(skin, {x, y, size, size}, unlocked ? WHITE : Color{105, 105, 105, 255});
    EndShaderMode();
}
// Authored stone texture with fixed-size bevels, even on shallow score strips.
inline void panel(Rectangle r, Color = WHITE) {
    float edge = std::min(26.f, r.height * .18f);
    float sx[] = {0, uiSlate.width * .12f, uiSlate.width * .88f, float(uiSlate.width)};
    float sy[] = {0, uiSlate.height * .15f, uiSlate.height * .85f, float(uiSlate.height)};
    float dx[] = {r.x, r.x + edge, r.x + r.width - edge, r.x + r.width};
    float dy[] = {r.y, r.y + edge, r.y + r.height - edge, r.y + r.height};
    for (int y = 0; y < 3; ++y)
        for (int x = 0; x < 3; ++x)
            DrawTexturePro(uiSlate, {sx[x], sy[y], sx[x + 1] - sx[x], sy[y + 1] - sy[y]},
                           {dx[x], dy[y], dx[x + 1] - dx[x], dy[y + 1] - dy[y]}, {0, 0}, 0, WHITE);
}
inline void modal(Rectangle r, const std::string &heading) {
    DrawRectangle(0, 0, 1280, 800, {0, 0, 0, 170});
    sprite(uiFrame, r);
    arcadeCenter(heading, r.x + r.width / 2, r.y + 66, 46, {170, 255, 64, 255});
}
inline void portraitCard(Texture2D portraits, int who, Rectangle r, const char *caption) {
    panel(r);
    float w = portraits.width / 2.f, h = portraits.height / 2.f;
    DrawTexturePro(portraits, {(who % 2) * w, (who / 2) * h, w, h},
                   {r.x + 14, r.y + 42, r.width - 28, r.height - 92}, {0, 0}, 0, WHITE);
    arcadeCenter(caption, r.x + r.width / 2, r.y + 16, 26, {170, 255, 64, 255});
    const char *names[] = {"YOU", "SLUGGER", "UMPIRE", "BALL BOY"};
    arcadeCenter(names[who], r.x + r.width / 2, r.y + r.height - 37, 23);
}
inline void baseballHud(Texture2D portraits, int rival, const std::array<bool, 10> &standing) {
    // Painted turf/rock cornice connects the shallow slate scoreboards.
    DrawTexturePro(uiFrame, {0, 0, float(uiFrame.width), uiFrame.height * .135f}, {0, 58, 1280, 55},
                   {0, 0}, 0, WHITE);
    DrawLineEx({558, 128}, {720, 65}, 12, {53, 33, 19, 255});
    DrawLineEx({560, 125}, {718, 62}, 7, {213, 170, 91, 255});
    DrawLineEx({561, 63}, {718, 128}, 12, {53, 33, 19, 255});
    DrawLineEx({563, 60}, {716, 125}, 7, {213, 170, 91, 255});
    DrawTriangle({640, 160}, {699, 83}, {581, 83}, {36, 52, 48, 255});
    DrawTriangleLines({640, 160}, {699, 83}, {581, 83}, gold);
    // The back row is at the top of the display, viewed from the approach.
    int pin = 0;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col <= row; ++col) {
            const float x = 640 + (row * .5f - col) * 19;
            const float y = 139 - row * 15;
            DrawCircle(x, y, 5, standing[pin++] ? Color{150, 242, 77, 255} : Color{78, 86, 77, 255});
        }
    arcadeCenter(std::to_string(std::count(standing.begin(), standing.end(), true)) + " STANDING", 640,
                 164, 19);
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
inline bool plaqueButton(Rectangle r, const std::string &s, bool enabled = true) {
    bool hover = enabled && !uiBlocked && CheckCollisionPointRec(GetMousePosition(), r);
    sprite(uiButton, r,
           enabled ? (hover ? Color{255, 255, 210, 255} : WHITE) : Color{100, 110, 100, 255});
    float size = std::min(30.f, (r.width - 46) /
                                    std::max(1.f, MeasureTextEx(arcadeFont, s.c_str(), 30, 1).x) * 30);
    arcadeCenter(s, r.x + r.width / 2, r.y + (r.height - size) / 2 - 2, size, enabled ? cream : GRAY);
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
