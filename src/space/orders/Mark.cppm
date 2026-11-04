
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>
#include <cstdint>

export module beatuy.feature.orders:mark;

import :book;
import beatuy.game;

export namespace feature {
class AttackMarks {
public:
    void begin();
    void show(uint32_t politicalId, void* target);
    void end();
    void clear();

private:
    struct Request {
        uint32_t politicalId;
        void* spatial;
    };
    struct Mark {
        uint32_t politicalId;
        size_t color;
        game::VisualEffect effect;
        bool shown;
    };

    void place(uint32_t politicalId, size_t color, void* spatial, float scale);

    std::array<Request, maximumEscort> requests{};
    size_t requestCount = 0;
    std::array<Mark, maximumEscort> marks{};
    size_t count = 0;
};
}

