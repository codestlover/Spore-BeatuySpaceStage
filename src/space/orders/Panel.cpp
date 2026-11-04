
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <span>
#include <utility>

module beatuy.feature.orders;

import beatuy.game;

namespace drawer {
namespace {
using feature::Order;

struct Settings {
    uint32_t pillFont = 0xaf14b67e, plainFont = 0x025a6d0f;
    std::array<uint32_t, game::Button::captionStates> pillCaptions{0xffa7d3ff, 0xff878087, 0xff94bce3, 0xffffffff,
                                                                   0xff362705, 0xff5d4309, 0xff362705, 0xffa7d3ff};
    game::Rectangle captionBorder{10.0f, 0.0f, 10.0f, 0.0f};
    std::array<float, 2> captionOffset{0.0f, 0.0f};
    uint32_t orderGroup = identifierPrefix | uint32_t(Control::Order) << 8 | 0xff;
    int pillWidth = 45, pillHeight = 22;
    uint32_t textWindowTable = 0x014453b0;
    const char16_t* tooltipLayout = u"Tooltips";
    uint32_t layoutGroup = 0x40464100, tooltipLayer = 0x05b598f6, layoutTable = 0x01418360;
    int tooltipFrameWidth = 50, tooltipFrameHeight = 20;
    uint32_t stoneHolder = 0x056cec38;
    int stoneSide = 44;
    float stoneScale = 0.56f, stoneEdge = 1.0f / 3;
    uint32_t openSound = 0x050130b7, closeSound = 0x98b8139d, pickSound = 0xefaaa0c7;
    float dismissReach = 42.0f, gap = 6.0f;
    float attachGap = 2.0f, slideFrom = -48.0f;
    float cubeSlideMilliseconds = 250.0f;
    game::Rectangle cubeArea{12.0f, 6.0f, 54.0f, 48.0f};
    float fallbackScale = 0.56f;
    float buttonHeight = 22.0f, lineStep = 24.0f;
    float rowWidth = 140.0f, itemWidth = 180.0f, shipWidth = 64.0f;
    float menuPadding = 7.0f, listGap = 6.0f, frameSide = 26.0f, shipStep = 28.0f;
    float pictureSide = 20.0f, pictureGap = 4.0f, menuGap = 4.0f, margin = 8.0f;
    uint32_t menuFill = 0xd0101820;
    float effectSeconds = 0.25f, glideEase = 0.2f, springDamping = 10.0f, fadeDamping = 0.1f;
    std::array<uint32_t, 3> buttonTables{0x01441390, 0x01441270, 0x01441200};
    std::array<uint32_t, 3> standardDrawableTables{0x01445238, 0x01445220, 0x014451e0};
    std::array<uint32_t, 2> glideTables{0x01442748, 0x01442810};
    uint32_t fadeTable = 0x01442478;
    std::array<float, 2> tooltipOffset{5.0f, -15.0f};
    bool pictures = true;
};
constexpr Settings settings{};

struct Look {
    uint32_t fill;
    std::array<uint32_t, game::Button::captionStates> captions;
};

constexpr std::array<Look, 2> looks{{
    {0xc0203048, {0xffa7d3ff, 0xff878087, 0xffffffff, 0xffffffff, 0xffa7d3ff, 0xff878087, 0xffffffff, 0xffffffff}},
    {0xe0d8a838, {0xff362705, 0xff5d4309, 0xff000000, 0xff000000, 0xff362705, 0xff5d4309, 0xff000000, 0xff000000}},
}};

constexpr std::array<const char16_t*, feature::maximumEscort> slotNumbers{
    u"1", u"2", u"3", u"4", u"5", u"6", u"7", u"8", u"9", u"10", u"11", u"12", u"13", u"14", u"15", u"16",
};

struct Screen {
    float top = 0.0f, height = 0.0f;
};

uint32_t identifier(Control control, size_t index) {
    return identifierPrefix | uint32_t(control) << 8 | uint32_t(index & 0xff);
}

bool gameFocused() {
    DWORD process = 0;
    HWND window = GetForegroundWindow();
    return window && GetWindowThreadProcessId(window, &process) && process == GetCurrentProcessId();
}

void play(uint32_t sound) {
    game::playAudio(sound);
}

uint32_t tableOf(const char* object) {
    return object ? game::gameAddress(game::virtualTable(object)) : 0;
}

const feature::OrderWords& wordsOf(Order order) {
    const auto& orders = feature::ordersTexts().orders;
    return size_t(order) < orders.size() ? orders[size_t(order)] : orders[0];
}

bool guardsEveryone(const feature::Standing& standing) {
    return standing.guardsAll;
}

game::Rectangle lineArea(float left, float right, size_t line) {
    const float top = settings.menuPadding + float(line) * settings.lineStep;
    return {left, top, right, top + settings.buttonHeight};
}

float shipTop(size_t place) {
    return settings.menuPadding + settings.lineStep + (settings.frameSide - settings.buttonHeight) / 2 +
           float(place) * settings.shipStep;
}

Screen screenOf(game::Window window) {
    Screen screen;
    for (int depth = 0; window && depth < 32; ++depth) {
        const game::Rectangle& area = *window.realArea();
        screen.top += area.top;
        screen.height = area.bottom - area.top;
        window = window.parent();
    }
    return screen;
}

game::Window newWindow(const game::Rectangle& area, uint32_t fill) {
    void* allocator = map::userInterface::windowAllocator();
    char* object = map::userInterface::windowNew(game::Window::objectByteSize, 4, "UI/Window", allocator);
    if (!object)
        return game::Window();
    map::userInterface::windowConstructor(object);
    const game::Window window(object + game::Window::faceOffset);
    window.addReference();
    window.setArea(area);
    window.setFill(fill);
    return window;
}

void discard(game::Window window) {
    if (!window)
        return;
    if (const game::Window parent = window.parent())
        parent.dispose(window);
    window.release();
}

template <std::derived_from<game::WindowEffect> Effect>
Effect newEffect(const char* name, const auto& constructor, uint32_t table) {
    void* allocator = map::userInterface::windowAllocator();
    char* memory = map::userInterface::windowNew(Effect::byteSize, game::WindowEffect::alignment, name, allocator);
    if (!memory)
        return Effect();
    char* made = constructor(memory);
    if (tableOf(made) != table)
        return Effect();
    const Effect effect(made);
    effect.addReference();
    return effect;
}

void setUp(game::WindowEffect effect, game::TwoStateEffect::Interpolation interpolation, float ease, float damping) {
    const game::TwoStateEffect face = effect.twoState();
    face.setTime(settings.effectSeconds);
    face.setEase(ease, ease);
    face.setInterpolation(interpolation);
    face.setDamping(damping);
    face.setTrigger(game::TwoStateEffect::Trigger::Invisible);
}

void addEffects(game::Window window, float offset) {
    using enum game::TwoStateEffect::Interpolation;
    const auto glide = newEffect<game::GlideEffect>("UI/GlideEffect", map::userInterface::glideEffectConstructor,
                                                    settings.glideTables[0]);
    const auto fade =
        newEffect<game::FadeEffect>("UI/FadeEffect", map::userInterface::fadeEffectConstructor, settings.fadeTable);
    const uint32_t offsetTable = glide ? tableOf(glide.raw() + game::GlideEffect::glideOffset) : 0;
    if (glide && offsetTable == settings.glideTables[1]) {
        setUp(glide, DampedSpring, settings.glideEase, settings.springDamping);
        glide.setOffset(offset, 0.0f);
        window.addProcedure(glide.raw());
    }
    if (glide)
        glide.release();
    if (fade) {
        setUp(fade, Linear, 0.0f, settings.fadeDamping);
        window.addProcedure(fade.raw());
        fade.release();
    }
}

void addTooltip(game::Window window, const char16_t* text) {
    void* allocator = map::userInterface::windowAllocator();
    char* memory = map::userInterface::windowNew(game::Tooltip::byteSize, 4, "UI/Tooltip", allocator);
    if (!memory)
        return;
    const game::Tooltip tooltip(map::userInterface::tooltipConstructor(
        memory, u"Tooltips", game::Tooltip::textControl, text, settings.tooltipOffset.data(), 0, u"", 0));
    if (!tooltip)
        return;
    tooltip.addReference();
    window.addProcedure(tooltip.raw());
    tooltip.release();
}

game::StandardDrawable newStandardDrawable() {
    void* allocator = map::userInterface::windowAllocator();
    char* memory = map::userInterface::windowNew(game::StandardDrawable::byteSize, 4, "UI/StdDrawable", allocator);
    char* made = memory ? map::userInterface::standardDrawableConstructor(memory) : nullptr;
    if (!made)
        return game::StandardDrawable();
    const std::array tables{tableOf(made), tableOf(made + game::StandardDrawable::layoutElementOffset),
                            tableOf(made + game::StandardDrawable::standardOffset)};
    if (tables != settings.standardDrawableTables)
        return game::StandardDrawable();
    const game::StandardDrawable drawable(made);
    drawable.addReference();
    return drawable;
}

NativeLook takeNativeLook(game::AlliesPanel panel, game::Window header) {
    NativeLook look;
    const game::Drawable drawable = header.drawable();
    const game::StandardDrawable pill = game::StandardDrawable::of(drawable);
    const game::InterfaceImage image = pill.image(0);
    if (image.sized(settings.pillWidth, settings.pillHeight)) {
        pill.addReference();
        look.pill = pill;
    }
    const game::TextWindow text(panel.findWindow(game::AlliesPanel::headerTextControl).raw());
    const uint32_t textTable = tableOf(text.raw());
    if (textTable == settings.textWindowTable) {
        look.outline = text.outline();
        look.outlined = true;
    }
    return look;
}

void seekStone(NativeLook& look, game::AlliesPanel panel) {
    if (look.stone || look.stoneSought)
        return;
    game::InterfaceImage stone;
    for (const game::AlliesSlot slot : panel.slots()) {
        if (stone || !slot || !slot.ship())
            continue;
        look.stoneSought = true;
        slot.layout().findWindow(settings.stoneHolder).forEachChild([&stone](game::Window child) {
            if (stone || child.controlIdentifier() != 0)
                return;
            const game::InterfaceImage image = game::StandardDrawable::of(child.drawable()).image(0);
            if (image.sized(settings.stoneSide, settings.stoneSide))
                stone = image;
        });
    }
    if (!look.stoneSought)
        return;
    look.stone = stone ? newStandardDrawable() : game::StandardDrawable();
    if (look.stone) {
        const float edge = settings.stoneEdge;
        look.stone.setImage(0, stone);
        look.stone.setScale(game::StandardDrawable::Scale::StretchMiddle);
        look.stone.setScaleArea({edge, edge, edge, edge});
        look.stone.setScaleFactor(settings.stoneScale, settings.stoneScale);
    }
}

void releaseLook(NativeLook& look) {
    if (look.pill)
        look.pill.release();
    if (look.stone)
        look.stone.release();
    look = NativeLook{};
}

TooltipFrame borrowTooltipFrame() {
    TooltipFrame tooltip{.tried = true};
    char* memory = map::application::allocate(game::Layout::byteSize, "UI/Tooltip Layout", 0, 0, nullptr, 0);
    char* made = memory ? map::userInterface::layoutConstructor(memory) : nullptr;
    const uint32_t layoutTable = tableOf(made);
    if (layoutTable == settings.layoutTable) {
        tooltip.layout = game::Layout(made);
        tooltip.layout.addReference();
    }
    const bool loaded =
        tooltip.layout.load(settings.tooltipLayout, settings.layoutGroup, false, settings.tooltipLayer);
    const game::Window text = loaded ? tooltip.layout.findWindow(game::Tooltip::textControl) : game::Window();
    const game::StandardDrawable frame = game::StandardDrawable::of(text ? text.drawable() : game::Drawable());
    const game::InterfaceImage image = frame.image(0);
    if (image.sized(settings.tooltipFrameWidth, settings.tooltipFrameHeight)) {
        frame.addReference();
        tooltip.frame = frame;
    }
    return tooltip;
}

void releaseTooltipFrame(TooltipFrame& tooltip) {
    if (tooltip.frame)
        tooltip.frame.release();
    if (tooltip.layout)
        tooltip.layout.release();
    tooltip = TooltipFrame{};
}

void dressPlain(const NativeLook&, game::Button, game::Button::Type) {}

void dressInPill(const NativeLook& look, game::Button button, game::Button::Type type) {
    button.window().setDrawable(look.pill);
    button.setType(type);
    if (type == game::Button::Type::Radio)
        button.setGroup(settings.orderGroup);
    for (uint32_t state = 0; state < settings.pillCaptions.size(); ++state)
        button.setCaptionColor(state, settings.pillCaptions[state]);
    button.setCaptionBorder(settings.captionBorder);
    button.setCaptionOffset(settings.captionOffset[0], settings.captionOffset[1]);
    if (look.outlined)
        button.setTextOutline(look.outline);
}

using Dressing = void (*)(const NativeLook&, game::Button, game::Button::Type);
constexpr std::array<Dressing, 2> dressings{dressPlain, dressInPill};

void applyPlainLook(game::Button button, bool checked) {
    const Look& look = looks[size_t(checked)];
    button.window().setFill(look.fill);
    for (uint32_t state = 0; state < look.captions.size(); ++state)
        button.setCaptionColor(state, look.captions[state]);
}

void applyPillLook(game::Button button, bool checked) {
    button.setStateFlag(game::Button::checkedFlag, checked);
}

using LookApplier = void (*)(game::Button, bool);
constexpr std::array<LookApplier, 2> lookAppliers{applyPlainLook, applyPillLook};

void applyLook(const NativeLook& look, game::Button button, bool checked) {
    if (button)
        lookAppliers[size_t(bool(look.pill))](button, checked);
}

game::Button newButton(const NativeLook& look, game::Window parent, uint32_t identifier, const char16_t* caption,
                       const char16_t* tooltip, const game::Rectangle& area, game::Button::Type type, bool checked) {
    void* allocator = map::userInterface::windowAllocator();
    char* memory = parent ? map::userInterface::windowNew(game::Button::byteSize, 4, "UI/WinButton", allocator)
                          : nullptr;
    char* made = memory ? map::userInterface::buttonConstructor(memory) : nullptr;
    if (!made)
        return game::Button();
    const std::array tables{tableOf(made), tableOf(made + game::Window::faceOffset),
                            tableOf(made + game::Button::buttonOffset)};
    if (tables != settings.buttonTables)
        return game::Button();
    const game::Button button(made);
    const game::Window window = button.window();
    window.addReference();
    window.setControlIdentifier(identifier);
    window.setCommandIdentifier(identifier);
    window.setFont(std::array{settings.plainFont, settings.pillFont}[size_t(bool(look.pill))]);
    window.setCaption(caption);
    dressings[size_t(bool(look.pill))](look, button, type);
    applyLook(look, button, checked);
    window.setArea(area);
    if (tooltip)
        addTooltip(window, tooltip);
    parent.add(window);
    return button;
}

game::Window newFrame(const NativeLook& look, game::Window parent, const game::Rectangle& area) {
    const game::Window window = parent ? newWindow(area, 0) : game::Window();
    if (!window)
        return window;
    window.setFlag(game::Window::ignoresMouseFlag, true);
    if (look.stone)
        window.setDrawable(look.stone);
    parent.add(window);
    return window;
}

game::Window newPicture(game::Window parent, const game::Key& picture, const game::Rectangle& area) {
    if (!settings.pictures || !picture.instance || !parent)
        return game::Window();
    const game::Window window = newWindow(area, 0);
    if (!window)
        return window;
    window.setFlag(game::Window::ignoresMouseFlag, true);
    parent.add(window);
    map::userInterface::setBackgroundByKey(window.raw(), &picture, 0);
    return window;
}
}
}

namespace feature {
using drawer::Control;

void OrdersPanel::tick(OrderBook& book, EscortReplies& replies) {
    update(book, replies);
}

void OrdersPanel::update(OrderBook& book, EscortReplies& replies) {
    if (!game::inSpace()) {
        if (session.panel) {
            restoreCubes();
            teardown();
        }
        worldClicked = false;
        return;
    }
    const game::AlliesPanel panel = game::SpaceInterface::get().alliesPanel();
    const game::Window container = panel.findWindow(game::AlliesPanel::containerControl);
    const game::Window header = panel.findWindow(game::AlliesPanel::headerControl);
    if (session.panel && (panel != session.panel || container != session.container)) {
        slideCount = 0;
        teardown();
    }
    if (!panel || !container || !header) {
        worldClicked = false;
        clicks.clear();
        return;
    }
    if (!session.panel) {
        if (panel == failedPanel)
            return;
        if (!build(panel, container, header)) {
            teardown();
            failedPanel = panel;
            return;
        }
    }
    handleClicks(book, replies);
    slideCubes(panel);
    const bool escape = drawer::gameFocused() && (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
    const bool escapePressed = escape && !escapeDown;
    escapeDown = escape;
    if (session.menuOpen && escapePressed) {
        closeMenu();
        drawer::play(drawer::settings.closeSound);
    }
    if (session.menuOpen && worldClicked)
        closeMenu();
    worldClicked = false;
    session.survey = surveyOf(panel);
    hideOutlying(panel);
    if (!rowsMatch(session.survey))
        rebuildRows(book);
    refreshRows(book);
    if (session.menuOpen && session.menu.revision != book.revision())
        refreshMenu(book);
    layout();
}

void OrdersPanel::onMouseDown(int) {
    worldClicked = true;
}

void OrdersPanel::close() {
    restoreCubes();
    teardown();
    drawer::releaseTooltipFrame(tooltip);
    openCount = 0;
    worldClicked = false;
    escapeDown = false;
    failedPanel = game::AlliesPanel();
    slideTicked = false;
}

bool OrdersPanel::build(game::AlliesPanel panel, game::Window container, game::Window header) {
    session.panel = panel;
    session.container = container;
    session.look = drawer::takeNativeLook(panel, header);
    drawer::seekStone(session.look, panel);
    if (!tooltip.tried)
        tooltip = drawer::borrowTooltipFrame();
    return true;
}

void OrdersPanel::teardown() {
    for (const Row& row : std::span(session.rows).first(session.rowCount))
        drawer::discard(row.button.window());
    session.rowCount = 0;
    closeMenu();
    drawer::releaseLook(session.look);
    session = Session{};
    clicks.clear();
}

void OrdersPanel::hideOutlying(game::AlliesPanel panel) {
    for (const game::AlliesSlot slot : panel.slots()) {
        if (!slot)
            continue;
        for (const uint32_t control : game::AlliesSlot::outlyingControls)
            if (const game::Window outlying = slot.layout().findWindow(control);
                outlying && (outlying.flags() & game::Window::visibleFlag))
                outlying.setFlag(game::Window::visibleFlag, false);
    }
}

void OrdersPanel::slideCubes(game::AlliesPanel panel) {
    const drawer::Settings& settings = drawer::settings;
    const uint32_t now = GetTickCount();
    const float elapsed = slideTicked ? float(now - lastSlideTick) : 0.0f;
    lastSlideTick = now;
    slideTicked = true;
    const float room = settings.rowWidth + 2 * settings.attachGap;
    std::array<Slide, maximumEscort> kept{};
    size_t keptCount = 0;
    for (const game::AlliesSlot slot : panel.slots()) {
        const game::Ship ship = slot ? slot.ship() : game::Ship();
        const game::Window root = ship ? slot.root() : game::Window();
        if (!root || keptCount == kept.size())
            continue;
        const uint32_t politicalId = ship.safePoliticalId();
        const std::span<Slide> known = std::span(slides).first(slideCount);
        const auto found = std::ranges::find(known, politicalId, &Slide::politicalId);
        Slide slide = found != known.end() && found->root == root ? *found : Slide{politicalId, root, 0.0f, 0.0f, 0.0f};
        const bool open = drawerOpen(politicalId);
        if (!open && slide.progress <= 0.0f && slide.applied == 0.0f)
            continue;
        const game::Rectangle area = *root.area();
        if (std::abs(area.left - (slide.home - slide.applied)) > 0.01f) {
            slide.home = area.left;
            slide.applied = 0.0f;
        }
        const float step = elapsed / settings.cubeSlideMilliseconds;
        slide.progress = std::clamp(slide.progress + (open ? step : -step), 0.0f, 1.0f);
        const float eased = slide.progress * slide.progress * (3.0f - 2.0f * slide.progress);
        const float offset = std::round(room * eased);
        if (offset != slide.applied) {
            root.setArea({slide.home - offset, area.top, slide.home - offset + (area.right - area.left), area.bottom});
            slide.applied = offset;
        }
        if (open || slide.applied != 0.0f)
            kept[keptCount++] = slide;
    }
    std::ranges::copy(std::span(kept).first(keptCount), slides.begin());
    slideCount = keptCount;
}

void OrdersPanel::restoreCubes() {
    for (const Slide& slide : std::span(slides).first(slideCount)) {
        if (!slide.root || slide.applied == 0.0f)
            continue;
        const game::Rectangle area = *slide.root.area();
        slide.root.setArea({slide.home, area.top, slide.home + (area.right - area.left), area.bottom});
    }
    slideCount = 0;
}

OrdersPanel::Survey OrdersPanel::surveyOf(game::AlliesPanel panel) const {
    const drawer::Settings& settings = drawer::settings;
    Survey survey;
    survey.slotsLeft = std::numeric_limits<float>::max();
    const float scale = panel.slotScale() > 0.0f && panel.slotScale() <= 1.0f ? panel.slotScale()
                                                                               : settings.fallbackScale;
    const uint32_t player = game::playerPoliticalId();
    size_t number = 0;
    for (const game::AlliesSlot slot : panel.slots()) {
        ++number;
        const game::Window root = slot ? slot.root() : game::Window();
        if (!root)
            continue;
        const game::Rectangle& area = *root.area();
        survey.slotsLeft = std::min(survey.slotsLeft, area.left);
        const game::Ship ship = slot.ship();
        if (!ship || survey.count >= survey.ships.size())
            continue;
        const uint32_t politicalId = ship.safePoliticalId();
        if (politicalId == player)
            continue;
        const game::Window picture = slot.layout().findWindow(game::AlliesSlot::pictureControl);
        const game::Rectangle cube = picture ? *picture.realArea() : settings.cubeArea;
        const float middleX = (area.left + area.right) / 2, middleY = (area.top + area.bottom) / 2;
        const float halfWidth = (area.right - area.left) / 2, halfHeight = (area.bottom - area.top) / 2;
        survey.ships[survey.count++] =
            ShipSlot{.politicalId = politicalId,
                     .number = number,
                     .root = root,
                     .middle = middleY + scale * ((cube.top + cube.bottom) / 2 - halfHeight),
                     .cubeRight = middleX + scale * (cube.right - halfWidth),
                     .picture = slot.picture()};
    }
    return survey;
}

bool OrdersPanel::rowsMatch(const Survey& survey) const {
    return std::ranges::equal(std::span(session.rows).first(session.rowCount),
                              std::span(survey.ships).first(survey.count), {}, &Row::politicalId,
                              &ShipSlot::politicalId) &&
           std::ranges::equal(std::span(session.rows).first(session.rowCount),
                              std::span(survey.ships).first(survey.count), {}, &Row::cube, &ShipSlot::root);
}

game::Button OrdersPanel::newRow(size_t index, uint32_t politicalId, Order order) {
    const OrderWords& words = drawer::wordsOf(order);
    const game::Button button = drawer::newButton(
        session.look, session.container, drawer::identifier(Control::Row, index), words.caption, words.description,
        game::Rectangle{}, game::Button::Type::Toggle, menuOpenFor(politicalId));
    if (!button)
        return button;
    const game::Window window = button.window();
    window.addProcedure(&rowProcedure);
    window.setFlag(game::Window::visibleFlag, false);
    drawer::addEffects(window, drawer::settings.slideFrom);
    return button;
}

bool OrdersPanel::menuOpenFor(uint32_t politicalId) const {
    return session.menuOpen && session.menu.politicalId == politicalId;
}

void OrdersPanel::markRows() {
    for (const Row& row : std::span(session.rows).first(session.rowCount))
        drawer::applyLook(session.look, row.button, menuOpenFor(row.politicalId));
}

bool OrdersPanel::drawerOpen(uint32_t politicalId) const {
    const auto open = std::span(openShips).first(openCount);
    return std::ranges::find(open, politicalId) != open.end();
}

void OrdersPanel::setDrawerOpen(uint32_t politicalId, bool open) {
    const std::span<uint32_t> kept = std::span(openShips).first(openCount);
    openCount -= std::ranges::remove(kept, politicalId).size();
    if (open && openCount < openShips.size())
        openShips[openCount++] = politicalId;
}

void OrdersPanel::showRows() {
    for (Row& row : std::span(session.rows).first(session.rowCount)) {
        const bool shown = drawerOpen(row.politicalId);
        if (shown == row.shown || !row.button)
            continue;
        row.button.window().setFlag(game::Window::visibleFlag, shown);
        row.shown = shown;
    }
}

void OrdersPanel::rebuildRows(const OrderBook& book) {
    closeMenu();
    for (const Row& row : std::span(session.rows).first(session.rowCount))
        drawer::discard(row.button.window());
    session.rowCount = 0;
    const Survey& survey = session.survey;
    const auto present = std::span(survey.ships).first(survey.count);
    const std::span<uint32_t> open = std::span(openShips).first(openCount);
    openCount -= std::ranges::remove_if(open, [&present](uint32_t politicalId) {
                     return std::ranges::find(present, politicalId, &ShipSlot::politicalId) == present.end();
                 }).size();
    for (size_t index = 0; index < survey.count; ++index) {
        const ShipSlot& ship = survey.ships[index];
        const Order order = book.orderOf(ship.politicalId);
        ship.root.addProcedure(&cubeProcedure);
        session.rows[index] = Row{ship.politicalId, order, newRow(index, ship.politicalId, order), ship.root, false};
    }
    session.rowCount = survey.count;
}

void OrdersPanel::refreshRows(const OrderBook& book) {
    for (size_t index = 0; index < session.rowCount; ++index) {
        Row& row = session.rows[index];
        const Order order = book.orderOf(row.politicalId);
        if (order == row.order)
            continue;
        drawer::discard(row.button.window());
        row.order = order;
        row.button = newRow(index, row.politicalId, order);
        if (row.shown && row.button)
            row.button.window().setFlag(game::Window::visibleFlag, true);
    }
}

void OrdersPanel::layout() {
    const drawer::Settings& settings = drawer::settings;
    const Survey& survey = session.survey;
    for (size_t index = 0; index < session.rowCount; ++index) {
        const game::Button button = session.rows[index].button;
        if (!button)
            continue;
        const float left = std::round(survey.ships[index].cubeRight + settings.attachGap);
        const float top = std::round(survey.ships[index].middle - settings.buttonHeight / 2);
        button.window().setArea({left, top, left + settings.rowWidth, top + settings.buttonHeight});
    }
    showRows();
    if (session.menuOpen)
        placeMenu();
}

void OrdersPanel::placeMenu() {
    const drawer::Settings& settings = drawer::settings;
    const Menu& menu = session.menu;
    const auto ships = std::span(session.survey.ships).first(session.survey.count);
    const auto found = std::ranges::find(ships, menu.politicalId, &ShipSlot::politicalId);
    if (found == ships.end()) {
        closeMenu();
        return;
    }
    const float right = std::round(session.survey.slotsLeft - settings.dismissReach - settings.gap);
    float top = found->middle - menu.height / 2;
    if (const drawer::Screen screen = drawer::screenOf(session.container); screen.height > 0) {
        const float highest = settings.margin - screen.top;
        top = std::clamp(top, highest, std::max(highest, screen.height - settings.margin - menu.height - screen.top));
    }
    top = std::round(top);
    menu.window.setArea({right - menu.width, top, right, top + menu.height});
}

void OrdersPanel::openMenu(const OrderBook& book, uint32_t politicalId) {
    Menu& menu = session.menu;
    menu = Menu{};
    menu.politicalId = politicalId;
    menu.revision = book.revision();
    const uint32_t fill = std::array{drawer::settings.menuFill, 0u}[size_t(bool(tooltip.frame))];
    menu.window = drawer::newWindow(game::Rectangle{}, fill);
    if (!menu.window)
        return;
    if (tooltip.frame)
        menu.window.setDrawable(tooltip.frame);
    menu.window.addProcedure(&menuProcedure);
    session.container.add(menu.window);
    session.container.toFront(menu.window);
    session.menuOpen = true;
    const Order current = book.orderOf(politicalId);
    for (size_t index = 0; index < orderCount; ++index) {
        const OrderWords& words = drawer::wordsOf(Order(index));
        menu.items[index] = drawer::newButton(session.look, menu.window, drawer::identifier(Control::Order, index),
                                              words.menuCaption, words.description, game::Rectangle{},
                                              game::Button::Type::Radio, Order(index) == current);
    }
    if (current == Order::ProtectEscort)
        openList(book);
    arrangeMenu();
    markRows();
}

void OrdersPanel::openList(const OrderBook& book) {
    const drawer::Settings& settings = drawer::settings;
    Menu& menu = session.menu;
    if (!session.menuOpen || menu.listOpen)
        return;
    menu.listOpen = true;
    drawer::seekStone(session.look, session.panel);
    const OrdersTranslation& texts = ordersTexts();
    const Standing standing = book.standingOf(menu.politicalId);
    const bool everyone = drawer::guardsEveryone(standing);
    const float frameLeft = settings.menuPadding, buttonLeft = frameLeft + settings.frameSide + settings.pictureGap;
    const float listRight = buttonLeft + settings.shipWidth;
    const float inset = (settings.frameSide - settings.pictureSide) / 2;
    menu.everyone =
        drawer::newButton(session.look, menu.window, drawer::identifier(Control::Everyone, 0), texts.everyone,
                          texts.everyoneDescription, drawer::lineArea(frameLeft, listRight, 0),
                          game::Button::Type::Toggle, everyone);
    for (const ShipSlot& ship : std::span(session.survey.ships).first(session.survey.count)) {
        if (ship.politicalId == menu.politicalId || menu.guardedCount >= menu.guarded.size())
            continue;
        const float frameTop = drawer::shipTop(menu.guardedCount);
        const float buttonTop = frameTop + (settings.frameSide - settings.buttonHeight) / 2;
        Guarded& guarded = menu.guarded[menu.guardedCount];
        guarded.politicalId = ship.politicalId;
        guarded.button = drawer::newButton(
            session.look, menu.window, drawer::identifier(Control::Guarded, menu.guardedCount),
            drawer::slotNumbers[std::clamp<size_t>(ship.number, 1, maximumEscort) - 1], nullptr,
            {buttonLeft, buttonTop, listRight, buttonTop + settings.buttonHeight}, game::Button::Type::Toggle,
            !everyone && standing.guards(ship.politicalId));
        guarded.frame = drawer::newFrame(
            session.look, menu.window,
            {frameLeft, frameTop, frameLeft + settings.frameSide, frameTop + settings.frameSide});
        guarded.picture = drawer::newPicture(
            guarded.frame, ship.picture, {inset, inset, inset + settings.pictureSide, inset + settings.pictureSide});
        ++menu.guardedCount;
    }
}

void OrdersPanel::arrangeMenu() {
    const drawer::Settings& settings = drawer::settings;
    Menu& menu = session.menu;
    const float listWidth = settings.frameSide + settings.pictureGap + settings.shipWidth + settings.listGap;
    const float ordersBottom = settings.menuPadding + float(orderCount - 1) * settings.lineStep + settings.buttonHeight;
    const float listBottom = menu.guardedCount > 0
                                 ? drawer::shipTop(menu.guardedCount - 1) + settings.frameSide
                                 : settings.menuPadding + settings.buttonHeight;
    menu.width = 2 * settings.menuPadding + (menu.listOpen ? listWidth : 0.0f) + settings.itemWidth;
    menu.height = std::max(ordersBottom, menu.listOpen ? listBottom : 0.0f) + settings.menuPadding;
    const float itemsLeft = menu.width - settings.menuPadding - settings.itemWidth;
    for (size_t index = 0; index < orderCount; ++index)
        if (const game::Button item = menu.items[index])
            item.window().setArea(drawer::lineArea(itemsLeft, itemsLeft + settings.itemWidth, index));
}

void OrdersPanel::refreshMenu(const OrderBook& book) {
    Menu& menu = session.menu;
    menu.revision = book.revision();
    const Standing standing = book.standingOf(menu.politicalId);
    const bool everyone = drawer::guardsEveryone(standing);
    for (size_t index = 0; index < orderCount; ++index)
        drawer::applyLook(session.look, menu.items[index], standing.order == Order(index));
    drawer::applyLook(session.look, menu.everyone, everyone);
    for (const Guarded& guarded : std::span(menu.guarded).first(menu.guardedCount))
        drawer::applyLook(session.look, guarded.button, !everyone && standing.guards(guarded.politicalId));
}

void OrdersPanel::closeMenu() {
    if (!session.menuOpen)
        return;
    Menu& menu = session.menu;
    for (const Guarded& guarded : std::span(menu.guarded).first(menu.guardedCount)) {
        drawer::discard(guarded.picture);
        drawer::discard(guarded.frame);
        drawer::discard(guarded.button.window());
    }
    drawer::discard(menu.everyone.window());
    for (const game::Button item : menu.items)
        drawer::discard(item.window());
    drawer::discard(menu.window);
    menu = Menu{};
    session.menuOpen = false;
    markRows();
}

void OrdersPanel::handleClicks(OrderBook& book, EscortReplies& replies) {
    using Handler = void (OrdersPanel::*)(OrderBook&, EscortReplies&, size_t);
    const std::array<Handler, drawer::controlCount> handlers{&OrdersPanel::onRow, &OrdersPanel::onOrder,
                                                            &OrdersPanel::onEveryone, &OrdersPanel::onGuarded};
    const drawer::Clicks taken = clicks;
    clicks.clear();
    const auto rows = std::span(session.rows).first(session.rowCount);
    for (const void* cube : std::span(taken.cubes).first(taken.cubeCount)) {
        const auto found = std::ranges::find(rows, cube, [](const Row& row) -> const void* { return row.cube.raw(); });
        if (found != rows.end())
            onCube(size_t(found - rows.begin()));
    }
    for (const uint32_t identifier : std::span(taken.identifiers).first(taken.count)) {
        const size_t control = (identifier >> 8) & 0xff, index = identifier & 0xff;
        if ((identifier & drawer::prefixMask) == drawer::identifierPrefix && control < handlers.size())
            (this->*handlers[control])(book, replies, index);
    }
    if (taken.count == 0)
        return;
    markRows();
    if (session.menuOpen)
        refreshMenu(book);
}

void OrdersPanel::onCube(size_t index) {
    const uint32_t politicalId = session.rows[index].politicalId;
    const bool open = !drawerOpen(politicalId);
    setDrawerOpen(politicalId, open);
    if (!open && menuOpenFor(politicalId))
        closeMenu();
    drawer::play(std::array{drawer::settings.closeSound, drawer::settings.openSound}[size_t(open)]);
}

void OrdersPanel::onRow(OrderBook& book, EscortReplies&, size_t index) {
    if (index >= session.rowCount)
        return;
    const uint32_t politicalId = session.rows[index].politicalId;
    const bool again = menuOpenFor(politicalId);
    closeMenu();
    if (!again)
        openMenu(book, politicalId);
    drawer::play(std::array{drawer::settings.openSound, drawer::settings.closeSound}[size_t(again)]);
}

void OrdersPanel::onOrder(OrderBook& book, EscortReplies& replies, size_t index) {
    if (!session.menuOpen || index >= orderCount)
        return;
    const Order order = Order(index);
    const uint32_t politicalId = session.menu.politicalId;
    const Standing before = book.standingOf(politicalId);
    book.setOrder(politicalId, order);
    replies.expect(politicalId, before);
    drawer::play(drawer::settings.pickSound);
    if (order != Order::ProtectEscort) {
        closeMenu();
        return;
    }
    openList(book);
    arrangeMenu();
}

void OrdersPanel::onEveryone(OrderBook& book, EscortReplies& replies, size_t) {
    if (!session.menuOpen || !session.menu.listOpen)
        return;
    const uint32_t politicalId = session.menu.politicalId;
    const Standing before = book.standingOf(politicalId);
    book.setGuarded(politicalId, {}, true);
    if (book.orderOf(politicalId) != Order::ProtectEscort)
        book.setOrder(politicalId, Order::ProtectEscort);
    replies.expect(politicalId, before);
    drawer::play(drawer::settings.openSound);
}

void OrdersPanel::onGuarded(OrderBook& book, EscortReplies& replies, size_t index) {
    const Menu& menu = session.menu;
    if (!session.menuOpen || !menu.listOpen || index >= menu.guardedCount)
        return;
    const uint32_t ship = menu.guarded[index].politicalId;
    const Standing standing = book.standingOf(menu.politicalId);
    const bool everyone = drawer::guardsEveryone(standing);
    const bool listed = !everyone && standing.guards(ship);
    std::array<uint32_t, maximumEscort> ships{};
    size_t count = 0;
    for (const uint32_t kept : everyone ? std::span<const uint32_t>() : standing.guardedShips())
        if (kept != ship && count < ships.size())
            ships[count++] = kept;
    if (!listed && count < ships.size())
        ships[count++] = ship;
    book.setGuarded(menu.politicalId, std::span<const uint32_t>(ships).first(count), count == 0);
    if (book.orderOf(menu.politicalId) != Order::ProtectEscort)
        book.setOrder(menu.politicalId, Order::ProtectEscort);
    replies.expect(menu.politicalId, standing);
    drawer::play(std::array{drawer::settings.openSound, drawer::settings.closeSound}[size_t(listed)]);
}
}

