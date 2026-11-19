#pragma once
// Canonical arcade layout, in shared simulation/render world units.
// Art themes may change decoration, never rescale these playable dimensions.
namespace bowling::geometry {
inline constexpr float width = 10.f, halfWidth = width / 2;
inline constexpr float foulLine = 2.f, surfaceStart = -7.f;
struct LaneLayout {
    float run;
    constexpr float at(float progress) const { return foulLine + run * progress; }
    constexpr float headPin() const { return at(1); }
    constexpr float deckEnd() const { return headPin() + 10.f; }
    constexpr float surfaceEnd() const { return deckEnd() + 3.f; }
    constexpr float sceneryExtension() const { return (headPin() - 44.f) / 2.f; }
};
inline constexpr LaneLayout stadiumLane{160.f}, mascotLane{144.f};
inline constexpr float headPin = stadiumLane.headPin(), deckEnd = stadiumLane.deckEnd();
inline constexpr float surfaceEnd = stadiumLane.surfaceEnd();
inline constexpr float ballRadius = .85f, heavyBallRadius = 1.1f, pinRadius = .55f;
inline constexpr float pinSpacingX = 2.65f, pinSpacingZ = 2.295f;
inline constexpr float pinHeight = 3.28f;
inline constexpr float launchMin = ballRadius + .1f, launchMax = width - launchMin;
inline constexpr float gutterWidth = .7f, gutterCenter = halfWidth + gutterWidth / 2;
inline constexpr float arrowZ = 12.f, dotsZ = 4.6f;
inline constexpr float sceneryDepthScale = 2.f; // decoration coordinates only
inline constexpr float sceneryExtension = stadiumLane.sceneryExtension();
inline constexpr float minLaunchSpeed = (headPin - foulLine) / 6.f;
inline constexpr float maxLaunchSpeed = (headPin - foulLine) / 3.5f;
inline constexpr float launchSpeed(float power) {
    return minLaunchSpeed + (maxLaunchSpeed - minLaunchSpeed) * power;
}
inline constexpr float shotTimeout = (deckEnd - foulLine) / minLaunchSpeed + 4.f;
inline constexpr float renderX(float x) {
    return halfWidth - x;
}
inline constexpr float rackX(int row, int col) {
    return halfWidth + (col - row * .5f) * pinSpacingX;
}
inline constexpr float rackZ(int row, LaneLayout lane = stadiumLane) {
    return lane.headPin() + row * pinSpacingZ;
}
struct CameraPreset {
    float height, back, targetHeight, fov;
};
inline constexpr CameraPreset stadiumCamera{3.8f, -4.5f, 1.1f, 55.f};
inline constexpr CameraPreset mascotCamera{3.70f, -7.45f, -13.35f, 53.f};
static_assert(rackX(3, 0) - pinRadius > 0 && rackX(3, 3) + pinRadius < width);
static_assert(rackZ(3) + pinRadius < deckEnd && deckEnd < surfaceEnd);
} // namespace bowling::geometry
