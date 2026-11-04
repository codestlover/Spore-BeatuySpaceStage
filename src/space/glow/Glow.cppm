
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>

export module beatuy.feature.glow;

export import :color;
import beatuy.game;

namespace feature {
export class Glow {
public:
    void tick();
    void onLeavePlanet(bool destroyed);
    void onSquad();
    void onAtmosphere(char* state, float altitude);
    void onLighting(char* state, float* model);
    void beginBake();
    void endBake();

private:
    struct War {
        float level = 0.0f;
        DWORD lastTick = 0;
    };
    struct Defense {
        DWORD nextProbe = 0;
        bool bar = false, calledOnce = false, secondPending = false;
        DWORD calledTick = 0, secondDue = 0, squadTick = 0;
        game::Planet squadPlanet;
    };
    struct Bakes {
        int depth = 0;
    };
    struct Change {
        char* state = nullptr;
        glow::Color ratio{1.0f, 1.0f, 1.0f};
    };

    void probe();
    [[nodiscard]] bool wanted();

    War war;
    Defense defense;
    Bakes bakes;
    Change change;
};
}

