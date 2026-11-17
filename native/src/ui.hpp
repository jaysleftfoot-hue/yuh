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
    DrawRectangleRounded({r.x + 3, r.y + 5, r.width, r.height}, .1f, 8, {0, 0, 0, 130});
    DrawRectangleRounded(r, .1f, 8, c);
    DrawRectangleRoundedLinesEx(r, .1f, 8, 3, {101, 126, 141, 220});
    DrawLineEx({r.x + 12, r.y + 3}, {r.x + r.width - 12, r.y + 3}, 2, {168, 184, 188, 160});
    DrawLineEx({r.x + 12, r.y + r.height - 3}, {r.x + r.width - 12, r.y + r.height - 3}, 2,
               {3, 9, 19, 230});
    for (float x : {r.x + 8, r.x + r.width - 8})
        for (float y : {r.y + 8, r.y + r.height - 8}) {
            DrawCircle(int(x), int(y), 2.3f, {89, 113, 127, 255});
            DrawCircle(int(x), int(y), 1, {17, 31, 43, 255});
        }
}
inline bool button(Rectangle r, const std::string &s, bool primary = false, bool enabled = true) {
    enabled = enabled && !uiBlocked;
    bool hover = enabled && CheckCollisionPointRec(GetMousePosition(), r);
    Color base = primary ? red : Color{31, 55, 80, 255};
    if (hover)
        base = primary ? Color{245, 75, 65, 255} : Color{49, 80, 110, 255};
    if (!enabled)
        base = {55, 62, 70, 255};
    DrawRectangleRounded({r.x, r.y + 5, r.width, r.height}, .2f, 8, {4, 9, 19, 255});
    DrawRectangleRounded(r, .2f, 8, base);
    DrawRectangleRoundedLinesEx(r, .2f, 8, 2, primary ? gold : Color{103, 136, 151, 255});
    DrawRectangleRounded({r.x + 5, r.y + 3, r.width - 10, r.height * .38f}, .2f, 8, Fade(WHITE, .10f));
    center(s, r.x + r.width / 2, r.y + (r.height - 20) / 2, 20, enabled ? cream : GRAY);
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
