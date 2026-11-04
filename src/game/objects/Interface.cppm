
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <concepts>
#include <cstdint>

export module beatuy.game.objects:interface;

import :ship;
import :view;
import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
struct Rectangle {
    float left, top, right, bottom;
};

class InterfaceImage : public View {
public:
    using View::View;
    int width() const {
        return object ? at<int>(0x1c) : 0;
    }
    int height() const {
        return object ? at<int>(0x20) : 0;
    }
    bool sized(int expectedWidth, int expectedHeight) const {
        return object && width() == expectedWidth && height() == expectedHeight;
    }
};

class Drawable : public View {
public:
    using View::View;
    void addReference() const {
        call<int>(0x0);
    }
    void release() const {
        call<int>(0x4);
    }
    char* cast(uint32_t type) const {
        return object ? call<char*>(0xc, type) : nullptr;
    }
};

class StandardDrawable : public Drawable {
public:
    using Drawable::Drawable;
    static constexpr uint32_t byteSize = 0x7c, layoutElementOffset = 0x4, standardOffset = 0xc;
    static constexpr uint32_t standardType = 0x103c1908;
    enum class Scale : uint32_t { Stretch = 1, StretchMiddle = 2, Tile = 3, TileMiddle = 4 };
    static StandardDrawable of(Drawable drawable) {
        return StandardDrawable(drawable.cast(standardType) ? drawable.raw() : nullptr);
    }
    InterfaceImage image(int index) const {
        char* face = standard();
        return InterfaceImage(face ? callVirtual<char*>(face, 0x10, index) : nullptr);
    }
    void* hitMask() const {
        char* face = standard();
        return face ? callVirtual<void*>(face, 0x30) : nullptr;
    }
    void setImage(int index, InterfaceImage image) const {
        if (char* face = standard())
            callVirtual<void>(face, 0x14, index, image.raw());
    }
    void setScale(Scale scale) const {
        if (char* face = standard())
            callVirtual<void>(face, 0x1c, uint32_t(scale));
    }
    void setScaleArea(const Rectangle& edges) const {
        if (char* face = standard())
            callVirtual<void>(face, 0x24, &edges);
    }
    void setScaleFactor(float across, float down) const {
        const float factor[2]{across, down};
        if (char* face = standard())
            callVirtual<void>(face, 0x2c, static_cast<const float*>(factor));
    }

private:
    char* standard() const {
        return cast(standardType);
    }
};

using TextOutline = std::array<uint32_t, 10>;

class Window : public View {
public:
    using View::View;
    static constexpr uint32_t objectByteSize = 0x20c, faceOffset = 4, visibleFlag = 1, ignoresMouseFlag = 0x10;
    static constexpr int maximumChildren = 256;
    void addReference() const {
        call<int>(0x0);
    }
    void release() const {
        call<int>(0x4);
    }
    Window parent() const {
        return Window(call<char*>(0x10));
    }
    uint32_t controlIdentifier() const {
        return call<uint32_t>(0x1c);
    }
    const Rectangle* area() const {
        return call<const Rectangle*>(0x34);
    }
    const Rectangle* realArea() const {
        return call<const Rectangle*>(0x38);
    }
    void setControlIdentifier(uint32_t identifier) const {
        call<void>(0x50, identifier);
    }
    void setCommandIdentifier(uint32_t identifier) const {
        call<void>(0x54, identifier);
    }
    void setArea(const Rectangle& area) const {
        call<void>(0x60, &area);
    }
    uint32_t flags() const {
        return call<uint32_t>(0x28);
    }
    void setFlag(uint32_t flag, bool on) const {
        call<void>(0x7c, flag, on);
    }
    void setCaption(const char16_t* caption) const {
        call<void>(0x80, caption);
    }
    void setFont(uint32_t font) const {
        call<void>(0x84, font);
    }
    Drawable drawable() const {
        return Drawable(call<char*>(0xa8));
    }
    void setDrawable(Drawable drawable) const {
        call<void>(0xb0, drawable.raw());
    }
    void setFill(uint32_t packedColor) const {
        call<void>(0xac, packedColor);
    }
    void add(Window child) const {
        call<void>(0xd8, child.raw());
    }
    void dispose(Window child) const {
        call<void>(0xe0, child.raw());
    }
    void toFront(Window child) const {
        call<void>(0xe8, child.raw());
    }
    void addProcedure(void* procedure) const {
        call<void>(0x104, procedure);
    }
    template <std::invocable<Window> Visit> void forEachChild(const Visit& visit) const {
        if (!object)
            return;
        char* const head = object + 0x38;
        char* link = field<char*>(head, 0);
        for (int count = 0; link && link != head && count < maximumChildren; ++count, link = field<char*>(link, 0))
            visit(Window(link - 4));
    }
};

class TextWindow : public Window {
public:
    using Window::Window;
    const TextOutline& outline() const {
        return at<TextOutline>(0x808);
    }
};

class Layout : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x18;
    void addReference() const {
        call<int>(0x4);
    }
    void release() const {
        call<int>(0x8);
    }
    bool load(const char16_t* name, uint32_t group, bool attach, uint32_t layer) const {
        return object && map::userInterface::layoutLoadByName(object, name, group, attach, layer);
    }
    Window findWindow(uint32_t control) const {
        return Window(object ? map::userInterface::findWindow(object, control, true) : nullptr);
    }
};

class Button : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x888, buttonOffset = 0x20c, captionStates = 8;
    enum class Type : uint32_t { Standard = 1, Toggle = 2, Radio = 3 };
    static constexpr uint32_t checkedFlag = 4;
    Window window() const {
        return part<Window>(Window::faceOffset);
    }
    void setType(Type type) const {
        callVirtual<void>(object + buttonOffset, 0x1c, uint32_t(type));
    }
    void setStateFlag(uint32_t flag, bool on) const {
        callVirtual<void>(object + buttonOffset, 0x28, flag, on);
    }
    void setGroup(uint32_t group) const {
        callVirtual<void>(object + buttonOffset, 0x44, group);
    }
    void setCaptionColor(uint32_t state, uint32_t packedColor) const {
        callVirtual<void>(object + buttonOffset, 0x4c, state, packedColor);
    }
    void setCaptionBorder(const Rectangle& border) const {
        callVirtual<void>(object + buttonOffset, 0x54, border.left, border.top, border.right, border.bottom);
    }
    void setCaptionOffset(float x, float y) const {
        callVirtual<void>(object + buttonOffset, 0x5c, x, y);
    }
    void setTextOutline(const TextOutline& outline) const {
        callVirtual<void>(object + buttonOffset, 0x6c, outline.data());
    }
};

class Tooltip : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x68, textControl = 0x03754e6c;
    void addReference() const {
        call<int>(0x0);
    }
    void release() const {
        call<int>(0x4);
    }
};

class AlliesSlot : public View {
public:
    using View::View;
    static constexpr uint32_t pictureType = 0x2f7d0004;
    Ship ship() const {
        return reference<Ship>(0x50);
    }
    Window root() const {
        return reference<Window>(0x13c);
    }
    Layout layout() const {
        return part<Layout>(0x7c);
    }
    static constexpr uint32_t pictureControl = 0x03d99151;
    static constexpr std::array<uint32_t, 2> outlyingControls{0x03d99185, 0x03d99163};
    Key picture() const {
        const Ship escort = ship();
        return escort ? Key{field<uint32_t>(escort.raw(), 0x780), pictureType, field<uint32_t>(escort.raw(), 0x788)}
                      : Key{};
    }
};

class AlliesPanel : public View {
public:
    using View::View;
    static constexpr uint32_t containerControl = 0x03d982ff, headerControl = 0x03dae412, headerTextControl = 0x064a90a0;
    Array<AlliesSlot*> slots() const {
        return arrayAt<AlliesSlot*>(0x0c);
    }
    Window findWindow(uint32_t control) const {
        return part<Layout>(0x1bc).findWindow(control);
    }
    float slotScale() const {
        return at<float>(0x1e8);
    }
};

class SpaceInterface : public View {
public:
    using View::View;
    static SpaceInterface get() {
        return SpaceInterface(map::userInterface::spaceGameInterface());
    }
    AlliesPanel alliesPanel() const {
        return reference<AlliesPanel>(0x5c4);
    }
};

class TwoStateEffect : public View {
public:
    using View::View;
    enum class Trigger : uint32_t { Visible = 0, Invisible = 1, Enabled = 2, Disabled = 3 };
    enum class Interpolation : uint32_t { Linear = 0, EaseInOut = 1, DampedSpring = 2 };
    void setTime(float seconds) const {
        call<void>(0x18, seconds);
    }
    void setTrigger(Trigger trigger) const {
        call<void>(0x20, uint32_t(trigger));
    }
    void setEase(float easeIn, float easeOut) const {
        call<void>(0x28, easeIn, easeOut);
    }
    void setInterpolation(Interpolation interpolation) const {
        call<void>(0x30, uint32_t(interpolation));
    }
    void setDamping(float damping) const {
        call<void>(0x38, damping);
    }
};

class WindowEffect : public View {
public:
    using View::View;
    static constexpr uint32_t alignment = 8, twoStateOffset = 0xc;
    void addReference() const {
        call<int>(0x0);
    }
    void release() const {
        call<int>(0x4);
    }
    TwoStateEffect twoState() const {
        return part<TwoStateEffect>(twoStateOffset);
    }
};

class InflateEffect : public WindowEffect {
public:
    using WindowEffect::WindowEffect;
    static constexpr uint32_t byteSize = 0xb8, scaleOffset = 0x60;
    void setScale(float scale) const {
        callVirtual<void>(object + scaleOffset, 0x18, scale);
    }
};

class FadeEffect : public WindowEffect {
public:
    using WindowEffect::WindowEffect;
    static constexpr uint32_t byteSize = 0x60;
};

class GlideEffect : public WindowEffect {
public:
    using WindowEffect::WindowEffect;
    static constexpr uint32_t byteSize = 0x70, glideOffset = 0x60;
    void setOffset(float x, float y) const {
        const float offset[2]{x, y};
        callVirtual<void>(object + glideOffset, 0x18, static_cast<const float*>(offset));
    }
};

class ColorSwatch : public View {
public:
    using View::View;
    bool& pressed() const {
        return at<bool>(0x0e);
    }
    bool& shadesOpen() const {
        return at<bool>(0x0f);
    }
    bool& flying() const {
        return at<bool>(0x10);
    }
    Array<ColorSwatch*> shades() const {
        return arrayAt<ColorSwatch*>(0x68);
    }
    Rectangle& area() const {
        return at<Rectangle>(0x3c);
    }
};

class ColorPicker : public View {
public:
    using View::View;
    static constexpr uint32_t byteSize = 0x50;
    void addReference() const {
        call<int>(0x4);
    }
    void release() const {
        call<int>(0x8);
    }
    char* objectFace() const {
        return object ? object + 0x8 : nullptr;
    }
    Array<ColorSwatch*> swatches() const {
        return arrayAt<ColorSwatch*>(0x30);
    }
    void*& configuration() const {
        return at<void*>(0x48);
    }
};

class PickedMessage : public View {
public:
    using View::View;
    uint32_t& colour() const {
        return at<uint32_t>(0x8);
    }
    char*& picker() const {
        return at<char*>(0x10);
    }
};

class InterfaceMessage : public View {
public:
    using View::View;
    enum class Type : int {
        KeyDown = 1,
        MouseDown = 6,
        MouseUp = 7,
        MouseMove = 8,
        MouseWheel = 9,
        ButtonClick = 0x17,
        ButtonSelect = 0x18,
        ComponentActivated = 0x287259f6,
    };
    Type& kind() const {
        return at<Type>(0x8);
    }
    int& key() const {
        return at<int>(0x10);
    }
    static constexpr int leftButton = 1000;
    int& mouseButton() const {
        return at<int>(0x18);
    }
    uint32_t& command() const {
        return at<uint32_t>(0xc);
    }
};
}

