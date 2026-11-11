
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdint>
#include <span>

export module beatuy.feature.colony;

export import :contact;
export import :crusade;
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
    void close();
    bool pardons(uint32_t who, uint32_t cause, map::identifiers::Relation event, float scale);
    std::span<game::RaidShip> lighten(const void* raid, char* list, std::span<game::RaidShip> changed) const;
    static void restore(std::span<const game::RaidShip> changed);
    void* aim(void* combatant, void* target) const;
    bool keepsGrox(char* civ) const;
    bool steer(char* ship, game::Vector& point) const;

private:
    static constexpr ColonySettings settings{};

    int aimTurrets(game::Ship ship, uint32_t player);

    DWORD nextAim = 0;
    Contact contact;
    Crusade crusade;
};
}

