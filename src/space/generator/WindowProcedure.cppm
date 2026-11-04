
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdint>

export module beatuy.feature.generator:windowprocedure;

import beatuy.game;

namespace picker {
struct WindowProcedure {
    static constexpr uint32_t windowProcedureType = 0x2f009dd0, objectType = 0xee3f516e, inputEvents = 0x01;

    bool* cancelled;

    explicit WindowProcedure(bool* flag) : cancelled(flag) {}

    virtual int addReference() {
        return 1;
    }
    virtual int release() {
        return 1;
    }
    virtual void* destroy(unsigned) {
        return this;
    }
    virtual void* cast(uint32_t type) const {
        return type == windowProcedureType || type == objectType ? const_cast<WindowProcedure*>(this) : nullptr;
    }
    virtual int getPriority() const {
        return 0;
    }
    virtual int getEventFlags() const {
        return inputEvents;
    }
    virtual bool handleInterfaceMessage(void*, const char* message) {
        using enum game::InterfaceMessage::Type;
        game::InterfaceMessage received(message);
        const game::InterfaceMessage::Type type = received.kind();
        if (type == KeyDown && received.key() == VK_ESCAPE) {
            *cancelled = true;
            return true;
        }
        return type >= MouseDown && type <= MouseWheel;
    }
};
}

