
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>
#include <cstdint>

export module beatuy.feature.orders:reply;

import :book;

export namespace feature {
class EscortReplies {
public:
    void expect(uint32_t politicalId, const Standing& before);
    void tick(const OrderBook& book);
    void reset();

private:
    struct Pending {
        uint32_t politicalId;
        Standing before;
    };
    struct Answer {
        uint32_t politicalId;
        uint32_t voice;
        uint32_t at;
    };

    void say(const Standing& standing, uint32_t now);

    std::array<Pending, maximumEscort> pending{};
    size_t pendingCount = 0;
    std::array<Answer, maximumEscort> answers{};
    size_t answerCount = 0;
};
}

