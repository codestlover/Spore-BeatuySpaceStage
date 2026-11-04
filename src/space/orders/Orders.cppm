
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>
#include <optional>

export module beatuy.feature.orders;

export import :behaviour;
export import :book;
export import :mark;
export import :panel;
export import :reply;
export import :saving;
export import :texts;

export namespace feature {
class Orders {
public:
    void tick();
    void onMouseDown(int button);
    void onLeavePlanet(bool destroyed);
    void close();
    void prepare();

    void onGameUpdate(uint32_t elapsedLowerWord);
    [[nodiscard]] void* target(void* combatant, void* target);
    [[nodiscard]] std::optional<void*> orderedTarget(void* combatant);
    void onHostile(void* combatant, void* attacker);
    void onRallyCall();
    [[nodiscard]] std::optional<bool> write(void* serializable, void* stream);
    [[nodiscard]] std::optional<bool> read(void* serializable, void* stream);

private:
    OrderBook book;
    OrdersPanel panel;
    EscortReplies replies;
    EscortBehaviour behaviour;
    OrdersSaving saving;
};
}

