
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <array>
#include <cstddef>
#include <cstdint>

export module beatuy.feature.orders:panel;

import :book;
import :reply;
import :texts;
import beatuy.game;

namespace drawer {
inline constexpr uint32_t identifierPrefix = 0xbea70000, prefixMask = 0xffff0000;

enum class Control : uint8_t { Row, Order, Everyone, Guarded };
inline constexpr size_t controlCount = size_t(Control::Guarded) + 1;

struct Clicks {
    std::array<uint32_t, 8> identifiers{};
    size_t count = 0;
    std::array<const void*, 8> cubes{};
    size_t cubeCount = 0;

    void add(uint32_t identifier) {
        if (count < identifiers.size())
            identifiers[count++] = identifier;
    }
    void addCube(const void* root) {
        if (cubeCount < cubes.size())
            cubes[cubeCount++] = root;
    }
    void clear() {
        count = cubeCount = 0;
    }
};

struct WindowProcedure {
    static constexpr uint32_t windowProcedureType = 0x2f009dd0, objectType = 0xee3f516e;
    static constexpr int inputEvents = 0x01, laterEvents = 0x80;

    Clicks* clicks;
    bool swallowsMouse;

    WindowProcedure(Clicks* queue, bool swallowing) : clicks(queue), swallowsMouse(swallowing) {}

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
        return inputEvents | laterEvents;
    }
    virtual bool handleInterfaceMessage(void*, const char* message) {
        using enum game::InterfaceMessage::Type;
        const game::InterfaceMessage received(message);
        const game::InterfaceMessage::Type type = received.kind();
        if (type == ComponentActivated || type == ButtonClick || type == ButtonSelect) {
            const uint32_t identifier = received.command();
            if ((identifier & prefixMask) != identifierPrefix)
                return false;
            if (type == ComponentActivated)
                clicks->add(identifier);
            return true;
        }
        return swallowsMouse && type >= MouseDown && type <= MouseWheel;
    }
};

struct CubeProcedure {
    Clicks* clicks;

    explicit CubeProcedure(Clicks* queue) : clicks(queue) {}

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
        using Procedure = WindowProcedure;
        return type == Procedure::windowProcedureType || type == Procedure::objectType
                   ? const_cast<CubeProcedure*>(this)
                   : nullptr;
    }
    virtual int getPriority() const {
        return 0;
    }
    virtual int getEventFlags() const {
        return WindowProcedure::inputEvents;
    }
    virtual bool handleInterfaceMessage(void* window, const char* message) {
        const game::InterfaceMessage received(message);
        if (received.kind() == game::InterfaceMessage::Type::MouseDown &&
            received.mouseButton() == game::InterfaceMessage::leftButton)
            clicks->addCube(window);
        return false;
    }
};

struct NativeLook {
    game::StandardDrawable pill;
    game::TextOutline outline{};
    bool outlined = false;
    game::StandardDrawable stone;
    bool stoneSought = false;
};

struct TooltipFrame {
    bool tried = false;
    game::Layout layout;
    game::StandardDrawable frame;
};
}

export namespace feature {
class OrdersPanel {
public:
    void tick(OrderBook& book, EscortReplies& replies);
    void onMouseDown(int button);
    void close();

private:
    struct ShipSlot {
        uint32_t politicalId = 0;
        size_t number = 0;
        game::Window root;
        float middle = 0.0f, cubeRight = 0.0f;
        game::Key picture{};
    };
    struct Survey {
        std::array<ShipSlot, maximumEscort> ships{};
        size_t count = 0;
        float slotsLeft = 0.0f;
    };
    struct Row {
        uint32_t politicalId = 0;
        Order order = Order::Free;
        game::Button button;
        game::Window cube;
        bool shown = false;
    };
    struct Guarded {
        uint32_t politicalId = 0;
        game::Button button;
        game::Window frame;
        game::Window picture;
    };
    struct Menu {
        uint32_t politicalId = 0;
        game::Window window;
        PerOrder<game::Button> items{};
        bool listOpen = false;
        game::Button everyone;
        std::array<Guarded, maximumEscort> guarded{};
        size_t guardedCount = 0;
        float width = 0.0f, height = 0.0f;
        uint32_t revision = 0;
    };
    struct Session {
        game::AlliesPanel panel;
        game::Window container;
        drawer::NativeLook look;
        std::array<Row, maximumEscort> rows{};
        size_t rowCount = 0;
        bool menuOpen = false;
        Menu menu;
        Survey survey;
    };
    struct Slide {
        uint32_t politicalId = 0;
        game::Window root;
        float home = 0.0f, applied = 0.0f, progress = 0.0f;
    };

    void update(OrderBook& book, EscortReplies& replies);
    bool build(game::AlliesPanel panel, game::Window container, game::Window header);
    void teardown();
    void slideCubes(game::AlliesPanel panel);
    void restoreCubes();
    static void hideOutlying(game::AlliesPanel panel);
    [[nodiscard]] Survey surveyOf(game::AlliesPanel panel) const;
    [[nodiscard]] bool rowsMatch(const Survey& survey) const;
    void rebuildRows(const OrderBook& book);
    void refreshRows(const OrderBook& book);
    [[nodiscard]] game::Button newRow(size_t index, uint32_t politicalId, Order order);
    [[nodiscard]] bool menuOpenFor(uint32_t politicalId) const;
    void markRows();
    [[nodiscard]] bool drawerOpen(uint32_t politicalId) const;
    void setDrawerOpen(uint32_t politicalId, bool open);
    void showRows();
    void layout();
    void placeMenu();

    void openMenu(const OrderBook& book, uint32_t politicalId);
    void openList(const OrderBook& book);
    void arrangeMenu();
    void refreshMenu(const OrderBook& book);
    void closeMenu();

    void handleClicks(OrderBook& book, EscortReplies& replies);
    void onCube(size_t index);
    void onRow(OrderBook& book, EscortReplies& replies, size_t index);
    void onOrder(OrderBook& book, EscortReplies& replies, size_t index);
    void onEveryone(OrderBook& book, EscortReplies& replies, size_t index);
    void onGuarded(OrderBook& book, EscortReplies& replies, size_t index);

    Session session;
    std::array<Slide, maximumEscort> slides{};
    size_t slideCount = 0;
    uint32_t lastSlideTick = 0;
    bool slideTicked = false;
    std::array<uint32_t, maximumEscort> openShips{};
    size_t openCount = 0;
    bool worldClicked = false, escapeDown = false;
    drawer::TooltipFrame tooltip;
    game::AlliesPanel failedPanel;
    drawer::Clicks clicks;
    drawer::WindowProcedure menuProcedure{&clicks, true}, rowProcedure{&clicks, false};
    drawer::CubeProcedure cubeProcedure{&clicks};
};
}

