#pragma once
#include "raylib.h"
#include <algorithm>
#include <string>
inline Font uiFont{};
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
    text(s, x + 3, y + 4, size, {0, 0, 0, 160});
    text(s, x, y, size, c);
}
inline void panel(Rectangle r, Color c) {
    DrawRectangleRec({r.x + 3, r.y + 5, r.width, r.height}, {0, 0, 0, 160});
    DrawRectangleGradientV(r.x, r.y, r.width, r.height, ColorBrightness(c, .35f),
                           ColorBrightness(c, -.45f));
    DrawRectangleLinesEx(r, 3, {160, 153, 119, 255});
    DrawRectangleLinesEx({r.x + 4, r.y + 4, r.width - 8, r.height - 8}, 1, {29, 32, 30, 255});
    // Brushed metal grain and inset rivets, like a physical arcade cabinet.
    for (int y = 8; y < r.height - 5; y += 7)
        DrawLine(r.x + 6, r.y + y, r.x + r.width - 6, r.y + y, {0, 0, 0, 8});
    for (float x : {r.x + 7, r.x + r.width - 7})
        for (float y : {r.y + 7, r.y + r.height - 7}) {
            DrawCircle(x, y, 3, {32, 34, 33, 255});
            DrawCircle(x - 1, y - 1, 1, {225, 214, 162, 255});
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
    Color base = primary ? Color{25, 130, 151, 255} : Color{24, 43, 61, 255};
    if (hover)
        base = primary ? Color{245, 75, 65, 255} : Color{49, 80, 110, 255};
    if (!enabled)
        base = {55, 62, 70, 255};
    panel(r, base);
    DrawLineEx({r.x + 9, r.y + 5}, {r.x + r.width - 9, r.y + 5}, 2, primary ? gold : cream);
    center(s, r.x + r.width / 2, r.y + (r.height - 16) / 2, 16,
           enabled ? cream : Color{140, 153, 164, 255});
    if (hover)
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}
inline void slider(const char *label, float x, float y, float width, float &value, float lo, float hi,
                   bool enabled) {
    text(label, x, y, 15, cream);
    text(std::string(TextFormat("%.0f", 100 * (value - lo) / (hi - lo))) + "%", x + width - 36, y, 15,
         gold);
    float t = (value - lo) / (hi - lo);
    DrawRectangleRounded({x, y + 29, width, 8}, 1, 8, {5, 15, 27, 255});
    DrawRectangleRounded({x, y + 29, width * t, 8}, 1, 8, enabled ? aqua : GRAY);
    DrawCircleV({x + t * width, y + 33}, 9, enabled ? cream : GRAY);
    DrawCircleV({x + t * width, y + 33}, 4, red);
    if (enabled && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(GetMousePosition(), {x - 10, y + 16, width + 20, 34}))
        value = lo + (hi - lo) * std::clamp((GetMouseX() - x) / width, 0.f, 1.f);
}

inline bool powerButton(int index, int stock, bool selected, bool enabled) {
    float x = 1182, y = 255 + index * 108.f;
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
