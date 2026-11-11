
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>

export module beatuy.feature.exit;

import beatuy.game;

namespace feature {
struct QuickExitSettings {
    float energyShare = 0.05f;
    float climb = -2.5f, climbSpeed = 2.0f;
    DWORD climbMilliseconds = 6000, climbEaseMilliseconds = 1200;
    DWORD cooldownMilliseconds = 1500;
    std::array<uint32_t, 16> rushPictures{0xd32238ca, 0xd32238c9, 0xd32238c8, 0xd32238cf, 0xd32238ce, 0xd32238cd,
                                          0xd32238cc, 0xd32238c3, 0xd32238c2, 0xd2223758, 0xd2223759, 0xd222375a,
                                          0xd222375b, 0xd222375c, 0xd222375d, 0xd222375e};
    uint32_t rushPictureType = 0x2f7d0004, rushPictureGroup = 0x3064cb38;
    float rushStep = 1.0905077f, rushPower = 3.0f;
    double rushFrameMilliseconds = 120.0;
    DWORD zoomRushMilliseconds = 800, rushHoldMilliseconds = 900, rushInMilliseconds = 250, rushOutMilliseconds = 600;
    uint32_t rushSound = 0x6043293c;
    int rootDepth = 32;
};

export class QuickExit {
public:
    bool onWheel(int delta);
    void tick();
    void close();
    [[nodiscard]] float speedOf(void* ship) const;

private:
    static constexpr QuickExitSettings settings{};

    bool request();
    bool zoomOut(DWORD now);
    bool climbOut(DWORD now);
    static bool pay();

    struct Rush {
        game::Window root, layer;
        std::array<game::StandardDrawable, settings.rushPictures.size()> looks{};
        std::array<void*, settings.rushPictures.size()> pictures{};
        size_t shown = settings.rushPictures.size();
        double drawn = 0.0;
        float light = 0.0f;
        DWORD until = 0;
    };

    void startRush(DWORD now, DWORD milliseconds);
    void holdRush(DWORD now);
    [[nodiscard]] game::StandardDrawable rushLook(size_t frame);
    void drawRush(DWORD now);
    void endRush();
    void sound(bool flying);

    struct Warp {
        int track = 0;
        bool heard = false;
    };

    bool climbing = false, keyDown = false;
    DWORD climbUntil = 0, lastRequest = 0;
    Rush rush;
    Warp warp;
};
}

