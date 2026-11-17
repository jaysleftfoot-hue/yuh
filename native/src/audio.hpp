#pragma once
#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <vector>
struct Audio {
    bool ready = false, muted = false;
    Sound hit{}, bowl{}, cheer{}, click{}, music{};
    Sound tone(float length, int kind) {
        int n = int(22050 * length);
        std::vector<float> data(n);
        std::mt19937 rng(143);
        std::uniform_real_distribution<float> noise(-1, 1);
        const float notes[] = {261.63f, 329.63f, 392.f, 523.25f, 440.f,   392.f, 329.63f, 293.66f,
                               261.63f, 329.63f, 392.f, 587.33f, 523.25f, 392.f, 329.63f, 293.66f};
        for (int i = 0; i < n; ++i) {
            float t = i / 22050.f, v = 0;
            if (kind == 0)
                v = noise(rng) * std::exp(-t * 12) * .6f + std::sin(t * 220) * std::exp(-t * 20) * .4f;
            if (kind == 1)
                v = noise(rng) * .18f * (1 - t / length) + std::sin(t * (180 - 30 * t)) * .08f;
            if (kind == 2) {
                float f =
                    std::array<float, 4>{523.25f, 659.25f, 783.99f, 1046.5f}[std::min(3, int(t / .18f))];
                v = std::sin(2 * PI * f * t) * .23f * std::min(1.f, (length - t) * 3) +
                    noise(rng) * .05f;
            }
            if (kind == 3)
                v = std::sin(2 * PI * (800 + 1000 * t) * t) * std::exp(-t * 40) * .25f;
            if (kind == 4) {
                float beat = t / .25f, phase = beat - std::floor(beat), f = notes[int(beat) % 16];
                v = (std::sin(2 * PI * f * t) + .3f * std::sin(4 * PI * f * t)) * .07f *
                    std::exp(-phase * 5);
                v += std::sin(2 * PI * (int(beat) % 4 < 2 ? 130.81f : 98.f) * t) * .055f +
                     noise(rng) * .03f * std::exp(-phase * 30);
            }
            data[i] = v * std::min(1.f, t * 100) * std::min(1.f, (length - t) * 100);
        }
        Wave w{static_cast<unsigned>(n), 22050, 32, 1, data.data()};
        return LoadSoundFromWave(w);
    }
    void init() {
        InitAudioDevice();
        ready = IsAudioDeviceReady();
        if (!ready)
            return;
        hit = tone(.35f, 0);
        bowl = tone(1.8f, 1);
        cheer = tone(1.2f, 2);
        click = tone(.12f, 3);
        music = tone(4, 4);
    }
    void play(Sound s) {
        if (ready && !muted)
            PlaySound(s);
    }
    void update() {
        if (ready && !muted && !IsSoundPlaying(music))
            PlaySound(music);
    }
    void toggle() {
        muted = !muted;
        if (ready) {
            SetMasterVolume(muted ? 0.f : 1.f);
            if (muted)
                StopSound(music);
        }
    }
    void close() {
        if (ready) {
            for (Sound s : {hit, bowl, cheer, click, music})
                UnloadSound(s);
            CloseAudioDevice();
            ready = false;
        }
    }
};
