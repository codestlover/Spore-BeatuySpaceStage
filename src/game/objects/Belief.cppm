
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>
#include <span>

export module beatuy.game.objects:belief;

import :view;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class BeliefPath : public View {
public:
    using View::View;
    std::span<Vector> points() const {
        return valuesAt<Vector>(0x10);
    }
    int& segment() const {
        return at<int>(0x30);
    }
    static constexpr uint32_t byteSize = 0x34;
};

class Belief : public View {
public:
    using View::View;
    static constexpr uint32_t projectileOffset = 0x504;
    static Belief ofProjectile(const void* projectile) {
        return Belief(projectile ? static_cast<const char*>(projectile) - projectileOffset : nullptr);
    }
    char* projectile() const {
        return object ? object + projectileOffset : nullptr;
    }
    BeliefPath path() const {
        return reference<BeliefPath>(0x518);
    }
    char*& target() const {
        return at<char*>(0x520);
    }
    Vector& aim() const {
        return at<Vector>(0x52c);
    }
    bool& timerRunning() const {
        return at<bool>(0x558);
    }
};
}

