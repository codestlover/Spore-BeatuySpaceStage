
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:event;

import :view;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
using EventClickFunction = char (*)(uint32_t event, void* user);
using EventRemovedFunction = void (*)(uint32_t event, void* user);

class EventItem : public View {
public:
    using View::View;
    float& fade() const {
        return at<float>(0x34);
    }
    float& removeAfter() const {
        return at<float>(0x38);
    }
    void*& user() const {
        return at<void*>(0x5c);
    }
    EventClickFunction& click() const {
        return at<EventClickFunction>(0x60);
    }
    EventRemovedFunction& removed() const {
        return at<EventRemovedFunction>(0x64);
    }
};
}

