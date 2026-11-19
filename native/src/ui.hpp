#pragma once
#include "campaign.hpp"
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
inline void rosterPortrait(Texture2D portraits, int who, Rectangle rect) {
    const float tile = portraits.width / 5.f;
    DrawTexturePro(portraits, {(who % 5) * tile, (who / 5) * tile, tile, tile}, rect, {0, 0}, 0, WHITE);
}
inline void portraitCard(Texture2D portraits, int who, Rectangle r, const char *caption) {
    panel(r);
    const float size = std::min(r.width - 28, r.height - 84);
    rosterPortrait(portraits, who, {r.x + (r.width - size) / 2, r.y + 41, size, size});
    arcadeCenter(caption, r.x + r.width / 2, r.y + 16, 26, {170, 255, 64, 255});
    arcadeCenter(bowling::roster[who].name, r.x + r.width / 2, r.y + r.height - 37, 23);
}
// Incandescent stadium numerals. Pitch controls both the bulb and the spacing.
inline void scoreboardLights(const std::string &value, float cx, float top, float pitch, Color lamp) {
    static constexpr unsigned char rows[10][7] = {
        {14,17,19,21,25,17,14}, {4,12,4,4,4,4,14}, {14,17,1,2,4,8,31},
        {30,1,1,14,1,1,30}, {2,6,10,18,31,2,2}, {31,16,16,30,1,1,30},
        {14,16,16,30,17,17,14}, {31,1,2,4,4,8,8}, {14,17,17,14,17,17,14},
        {14,17,17,15,1,1,14}
    };
    float left = cx - (value.size() * 6.f - 2.f) * pitch / 2;
    for (char digit : value) {
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col) {
                if (!(rows[digit - '0'][row] & (16 >> col)))
                    continue;
                Vector2 p{left + col * pitch, top + row * pitch};
                DrawCircleV(p, pitch * .54f, ColorAlpha(lamp, .32f));
                DrawCircleV(p, pitch * .41f, lamp);
            }
        left += 6 * pitch;
    }
}
inline void baseballHud(Texture2D portraits, Texture2D pinGauge, int player, int rival, const std::array<bool, 10> &standing) {
    // Painted turf/rock cornice connects the shallow slate scoreboards.
    DrawTexturePro(uiFrame, {0, 0, float(uiFrame.width), uiFrame.height * .135f}, {0, 58, 1280, 55},
                   {0, 0}, 0, WHITE);
    sprite(pinGauge, {543, 61, 194, 103});
    // The back row is at the top of the display, viewed from the approach.
    int pin = 0;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col <= row; ++col) {
            const float x = 640 + (row * .5f - col) * 19;
            const float y = 139 - row * 15;
            const bool lit = standing[pin++];
            DrawCircleV({x, y}, 6.5f, {41, 31, 17, 255});
            DrawCircleV({x, y - .5f}, 5.f, lit ? Color{154, 225, 74, 255} : Color{65, 70, 55, 255});
            if (lit)
                DrawCircleV({x - 1, y - 1.5f}, 2.f, {233, 255, 173, 255});
        }
    arcadeCenter(std::to_string(std::count(standing.begin(), standing.end(), true)) + " STANDING", 640,
                 164, 19);
    for (int side = 0; side < 2; ++side) {
        float x = side ? 1206 : 80;
        int who = side ? rival : player;
        panel({x - 68, 128, 136, 138}, {54, 70, 76, 255});
        rosterPortrait(portraits, who, {x - 56, 132, 112, 112});
        const float size = std::min(20.f, 122.f * 20 / MeasureTextEx(arcadeFont, bowling::roster[who].name, 20, 1).x);
        arcadeCenter(bowling::roster[who].name, x, 244, size, cream);
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
