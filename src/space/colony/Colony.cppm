
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdint>

export module beatuy.feature.colony;

export import :contact;
import beatuy.game;

namespace feature {
struct ColonySettings {
    DWORD aimMilliseconds = 500;
    float reach = 400.0f;
};

export class Colony {
public:
    void tick();
    void onContact(void* screen, uint32_t politicalId);
    void onContactUnload();

private:
    static constexpr ColonySettings settings{};

    int aimTurrets(game::Ship ship, uint32_t player);

    DWORD nextAim = 0;
    Contact contact;
};
}

