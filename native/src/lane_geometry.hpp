#pragma once
// Canonical arcade layout, in shared simulation/render world units.
// Art themes may change decoration, never rescale these playable dimensions.
namespace bowling::geometry {
inline constexpr float width = 10.f, halfWidth = width / 2;
inline constexpr float foulLine = 2.f, headPin = 82.f, deckEnd = headPin + 10.f;
inline constexpr float surfaceStart = -7.f, surfaceEnd = deckEnd + 3.f;
inline constexpr float surfaceLength = surfaceEnd - surfaceStart;
inline constexpr float surfaceCenter = (surfaceStart + surfaceEnd) / 2;
inline constexpr float ballRadius = .85f, heavyBallRadius = 1.1f, pinRadius = .55f;
inline constexpr float pinSpacingX = 2.65f, pinSpacingZ = 2.295f;
inline constexpr float pinHeight = 3.28f;
inline constexpr float launchMin = ballRadius + .1f, launchMax = width - launchMin;
inline constexpr float gutterWidth = .7f, gutterCenter = halfWidth + gutterWidth / 2;
inline constexpr float arrowZ = 12.f, dotsZ = 4.6f;
inline constexpr float sceneryDepthScale = 2.f; // decoration coordinates only
inline constexpr float sceneryExtension = (headPin - 44.f) / sceneryDepthScale;
inline constexpr float shotTimeout = (deckEnd - foulLine) / 11.f + 4.f;
inline constexpr float renderX(float x) {
    return halfWidth - x;
}
inline constexpr float rackX(int row, int col) {
    return halfWidth + (col - row * .5f) * pinSpacingX;
}
inline constexpr float rackZ(int row) {
    return headPin + row * pinSpacingZ;
}
struct CameraPreset {
    float height, back, targetHeight, fov;
};
inline constexpr CameraPreset stadiumCamera{3.8f, -4.5f, 1.1f, 55.f};
inline constexpr CameraPreset mascotCamera{4.f, -7.f, -1.f, 53.f};
static_assert(rackX(3, 0) - pinRadius > 0 && rackX(3, 3) + pinRadius < width);
static_assert(rackZ(3) + pinRadius < deckEnd && deckEnd < surfaceEnd);
} // namespace bowling::geometry
