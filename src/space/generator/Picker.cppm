
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>
#include <cstdint>

export module beatuy.feature.generator:picker;

import :windowprocedure;
import beatuy.game;

namespace picker {
struct Settling {
    bool pressed = false, shadesOpen = false, flying = false;

    [[nodiscard]] bool holds() const {
        return pressed || shadesOpen || flying;
    }
};
}

namespace feature {
export class Picker {
public:
    using Chosen = void (*)(uint32_t packedColor, void* context);
    bool open(const float color[3], const game::Vector& place, Chosen chosen, void* context);
    void cancel();
    void close();
    [[nodiscard]] bool reaches(const game::Vector& place) const;
    [[nodiscard]] bool isOpen() const;
    void tick();

private:
    struct Session {
        bool pending = false;
        std::array<float, 3> color{1.0f, 1.0f, 1.0f};
        game::Vector place{};
        Chosen chosen = nullptr;
        void* context = nullptr;
        game::Window layer, host, container;
        float width = 0.0f, height = 0.0f, scale = 1.0f;
        std::array<game::Rectangle, 32> swatchAreas{};
        size_t swatchCount = 0;
        bool escapeDown = false;
        game::ColorPicker picker;
        bool handler = false, shown = false;
        bool cancelled = false, picked = false, newPick = false;
        uint32_t packedColor = 0;
        DWORD lastTick = 0;
        DWORD settleFrom = 0;
        bool hiding = false;
        DWORD closeAt = 0;
    };

    static void __cdecl onMessage(uint32_t identifier, void* message, void* data);
    void receive(uint32_t identifier, void* message);
    void deliverPick(DWORD now);
    void settle(DWORD now);
    void hide(DWORD now);
    void teardown();
    bool build();
    bool follow();
    void resize(float scale);

    Session session;
    picker::WindowProcedure hostProcedure{&session.cancelled};
};
}

