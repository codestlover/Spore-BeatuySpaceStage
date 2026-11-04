
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>

export module beatuy.game.objects:application;

import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class CameraManager : public View {
public:
    using View::View;
    char* camera(map::identifiers::Type type) const {
        void* active = object ? call<void*>(0x38) : nullptr;
        return active ? callVirtual<char*>(active, 0x0c, uint32_t(type)) : nullptr;
    }
    bool knowsViewer() const {
        return object && virtualTable(object)[0x1c / 4] == map::application::cameraManagerViewer.at();
    }
    char* viewer() const {
        return call<char*>(0x1c);
    }
};

class GameModes : public View {
public:
    using View::View;
    static GameModes get() {
        return GameModes(map::application::gameModeManager());
    }
    map::identifiers::Mode mode() const {
        return call<map::identifiers::Mode>(0x38);
    }
    CameraManager cameras() const {
        return CameraManager(object ? call<void*>(0x50) : nullptr);
    }
};

class MessageManager : public View {
public:
    using View::View;
    using Handler = void(__cdecl*)(uint32_t identifier, void* message, void* data);
    static MessageManager get() {
        return MessageManager(map::application::messageManager());
    }
    void addHandler(Handler handler, uint32_t identifier) const {
        call<void>(0x28, reinterpret_cast<void*>(handler), static_cast<void*>(nullptr), identifier, 0, -1);
    }
    void removeHandler(Handler handler, uint32_t identifier) const {
        call<void>(0x30, reinterpret_cast<void*>(handler), identifier, -9999);
    }
};

class VisualEffect : public View {
public:
    using View::View;
    void start() const {
        call<bool>(0x8, 0);
    }
    void stop() const {
        call<bool>(0xc, 0);
    }
    void setSourceTransform(const void* transform) const {
        call<int>(0x18, transform);
    }
    void release() const {
        call<int>(0x4);
    }
};

class EffectsManager : public View {
public:
    using View::View;
    static EffectsManager get() {
        return EffectsManager(map::application::effectsManager());
    }
    VisualEffect create(uint32_t identifier) const {
        char* made = nullptr;
        return object && call<bool>(0x2c, identifier, 0, &made) ? VisualEffect(made) : VisualEffect();
    }
};
}

