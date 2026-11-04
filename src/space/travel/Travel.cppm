
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>

export module beatuy.feature.travel;

import beatuy.game;

namespace feature {
struct TravelSettings {
    DWORD echoMilliseconds = 40;
    DWORD travelMilliseconds = 120000, enterMilliseconds = 8000;
    LONG slopPixels = 6;
};

export class Travel {
public:
    void onMouseDown(int button);
    void tick();

private:
    struct Click {
        void* target = nullptr;
        map::identifiers::Context context = map::identifiers::Context::None;
        DWORD tick = 0;
        POINT at{};
    };
    static constexpr TravelSettings settings{};

    void disarm();
    [[nodiscard]] static bool nearby(const POINT& first, const POINT& second);

    Click last;
    void* armed = nullptr;
    map::identifiers::Context armedContext = map::identifiers::Context::None;
    DWORD deadline = 0;
    bool zoomed = false;
};
}

