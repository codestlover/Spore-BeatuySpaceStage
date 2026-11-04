
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>

export module beatuy.feature.exit;

namespace feature {
struct QuickExitSettings {
    float energyShare = 0.05f;
    float climb = -2.5f;
    DWORD climbMilliseconds = 6000;
    DWORD cooldownMilliseconds = 1500;
};

export class QuickExit {
public:
    bool onWheel(int delta);
    void tick();

private:
    static constexpr QuickExitSettings settings{};

    bool request();
    bool zoomOut(DWORD now);
    bool climbOut(DWORD now);
    static bool pay();

    bool climbing = false, keyDown = false;
    DWORD climbUntil = 0, lastRequest = 0;
};
}

