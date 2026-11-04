
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

export module beatuy.feature.orders:saving;

import :book;
import beatuy.game;

export namespace feature {
struct EmpireFingerprint {
    uint32_t politicalId = 0;
    uint32_t homeStar = 0;
    uint32_t homePlanet = 0;

    [[nodiscard]] bool known() const {
        return politicalId || homeStar || homePlanet;
    }
    friend bool operator==(const EmpireFingerprint&, const EmpireFingerprint&) = default;
};

class OrdersSaving {
public:
    bool prepare(OrderBook& book);
    [[nodiscard]] std::optional<bool> write(void* serializable, void* stream);
    [[nodiscard]] std::optional<bool> read(void* serializable, void* stream);
    void tick(OrderBook& book);

private:
    static void* __cdecl attributeData(void* attribute);
    static bool __cdecl writeAttribute(void* stream, void* data);
    static bool __cdecl readAttribute(void* stream, void* data);
    static void __cdecl attributeMissing(void* attribute);

    bool serialize(void* serializable, void* stream,
                   const map::Function<bool(__thiscall*)(void*, void*)>& serializerFunction);
    size_t compose(std::span<uint32_t> words) const;
    void load(std::span<const uint32_t> payload);
    void forget();
    void dropForeign(OrderBook& orderBook);

    OrderBook* book = nullptr;
    bool ready = false;
    EmpireFingerprint givenFor;
    game::PlayerShipManager record;
};
}

