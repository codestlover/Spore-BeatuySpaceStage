
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>

module beatuy.game.world;

namespace game {
namespace effects {
struct Transform {
    uint16_t flags;
    uint16_t count;
    std::array<float, 3> position;
    float scale;
    std::array<float, 9> rotation;
};
static_assert(sizeof(Transform) == 0x38 && offsetof(Transform, scale) == 0x10);

struct Catcher {
    Transform caught{};
    bool kept = false;

    virtual int addReference() {
        return 1;
    }
    virtual int release() {
        return 1;
    }
    virtual bool start(int) {
        return true;
    }
    virtual bool stop(int) {
        return true;
    }
    virtual bool running() {
        return true;
    }
    virtual void reserved() {}
    virtual void setSourceTransform(const Transform* transform) {
        caught = *transform;
        kept = true;
    }
};
}

namespace {
namespace application = map::application;
namespace space = map::space;
namespace planet = map::planet;
namespace civilization = map::civilization;
namespace userInterface = map::userInterface;

class SpacePlayerData : public View {
public:
    using View::View;
    Planet activePlanet() const {
        return reference<Planet>(0x4);
    }
    View star() const {
        return reference<View>(0x8);
    }
    map::identifiers::Context& context() const {
        return at<map::identifiers::Context>(0x10);
    }
    uint32_t& empirePoliticalId() const {
        return at<uint32_t>(0x18);
    }
};

class Star : public View {
public:
    using View::View;
    View record() const {
        return reference<View>(0x48);
    }
};

class TimeManager : public View {
public:
    using View::View;
    bool paused() const {
        return at<uint8_t>(0x48) & 1;
    }
};

class Relationship : public View {
public:
    using View::View;
    float& standing() const {
        return at<float>(0x0);
    }
    uint32_t& flags() const {
        return at<uint32_t>(0x4);
    }
};

SpacePlayerData playerData() {
    return SpacePlayerData(*space::spacePlayerData);
}

bool leaving = false;
Planet leftPlanet;

Relationship relationData(uint32_t who, uint32_t towards) {
    void* relationships = civilization::relations();
    if (!relationships)
        return Relationship();
    void* table = civilization::relationshipMap(relationships, who, towards);
    return Relationship(civilization::relationshipData(relationships, table, who, towards));
}

Camera activeCamera(map::identifiers::Type type) {
    return Camera(GameModes::get().cameras().camera(type));
}
}

bool inSpace() {
    GameModes modes = GameModes::get();
    return modes && modes.mode() == map::identifiers::Mode::Space && playerData();
}

map::identifiers::Context context() {
    SpacePlayerData data = playerData();
    return data ? data.context() : map::identifiers::Context::None;
}

void markLeaving() {
    leaving = true;
    leftPlanet = activePlanet();
}

bool onPlanet() {
    if (!inSpace() || context() != map::identifiers::Context::Planet) {
        leaving = false;
        return false;
    }
    if (leaving && activePlanet() != leftPlanet)
        leaving = false;
    return !leaving;
}

bool paused() {
    TimeManager time(application::timeManager());
    return time && time.paused();
}

PlayerShipManager shipManager() {
    return PlayerShipManager(*space::playerShipManager);
}

Ship playerShip() {
    PlayerShipManager manager = shipManager();
    return manager ? manager.ship() : Ship();
}

Vector shipPosition() {
    const Vector* position = playerShip().body().position();
    return position ? *position : Vector{0.0f, 0.0f, 0.0f};
}

uint32_t playerPoliticalId() {
    SpacePlayerData data = playerData();
    return data ? data.empirePoliticalId() : uint32_t(-1);
}

Empire playerEmpire() {
    return Empire(space::playerEmpire());
}

Empire empireOf(uint32_t politicalId) {
    char* stars = space::starManager();
    return Empire(stars ? space::empireOf(stars, politicalId) : nullptr);
}

size_t identityColor(uint32_t politicalId) {
    static constexpr uint32_t firstIdentifier = 0x053dbcf1;
    static constexpr std::array<std::array<float, 3>, identityColors> colors{{
        {0.5f, 0.5f, 0.5f},
        {0.685f, 0.425f, 0.0f},
        {0.878f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.878f},
        {0.474f, 0.871f, 0.031f},
        {0.62f, 0.539f, 1.0f},
        {1.0f, 0.663f, 0.173f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 0.08f, 0.25f},
        {0.0f, 0.46f, 1.0f},
        {0.0f, 0.395f, 0.009f},
        {0.562f, 0.0f, 0.0f},
        {0.294f, 0.08f, 0.88f},
    }};
    Empire empire = empireOf(politicalId);
    if (!empire)
        return 0;
    uint32_t place = empire.colorIdentifier() - firstIdentifier - 1;
    if (place < identityColors)
        return place;
    std::array<float, 3> shown{};
    if (!empire.colour(shown))
        return 0;
    auto distance = [&shown](const std::array<float, 3>& color) {
        float sum = 0.0f;
        for (size_t channel = 0; channel < color.size(); ++channel)
            sum += (color[channel] - shown[channel]) * (color[channel] - shown[channel]);
        return sum;
    };
    return size_t(std::ranges::min_element(colors, {}, distance) - colors.begin());
}

std::optional<int> addPlayerMoney(int amount) {
    Empire empire = playerEmpire();
    if (!empire)
        return std::nullopt;
    return empire.money() += amount;
}

Planet activePlanet() {
    SpacePlayerData data = playerData();
    return data ? data.activePlanet() : Planet();
}

PlanetRecord activeRecord() {
    Planet planet = activePlanet();
    return planet ? planet.record() : PlanetRecord();
}

map::identifiers::Technology planetTechnology() {
    PlanetRecord record = activeRecord();
    return record ? record.technology() : map::identifiers::Technology::None;
}

Empire planetOwner() {
    Planet planet = activePlanet();
    return Empire(planet ? planet::owner(planet.raw()) : nullptr);
}

uint32_t planetOwnerPoliticalId() {
    Empire owner = planetOwner();
    return owner ? owner.politicalId() : uint32_t(-1);
}

Capture planetCapture() {
    Capture progress{0.0f, uint32_t(-1)};
    if (PlanetRecord record = activeRecord())
        progress.percent = planet::capture(record.raw(), &progress.capturer);
    return progress;
}

StarRecord activeStarRecord() {
    SpacePlayerData data = playerData();
    Star star(data ? data.star().raw() : nullptr);
    return StarRecord(star ? star.record().raw() : nullptr);
}

View hoveredStar() {
    PlayerShipManager manager = shipManager();
    return View(manager ? space::hoveredStar(manager.raw()) : nullptr);
}

GameData hoveredObject() {
    void* views = application::gameViewManager();
    return GameData(views ? field<void*>(views, 0x88) : nullptr);
}

bool canEnterPlanet() {
    return space::canEnterPlanet();
}

bool canEnterSystem() {
    return space::canEnterSystem();
}

float minimumZoom(map::identifiers::Context level) {
    return space::minimumZoom(int(level));
}

float maximumZoom(map::identifiers::Context level) {
    return space::maximumZoom(int(level));
}

Camera spaceCamera() {
    return activeCamera(map::identifiers::Type::SpaceCamera);
}

Camera planetCamera() {
    return activeCamera(map::identifiers::Type::PlanetCamera);
}

Viewer activeViewer() {
    CameraManager cameras = GameModes::get().cameras();
    return cameras && cameras.knowsViewer() ? Viewer(cameras.viewer()) : Viewer();
}

namespace {
Vector normalized(const Vector& direction, const Vector& fallback = {0, 0, 1}) {
    float magnitude = length(direction);
    return magnitude > 1e-6f ? direction * (1.0f / magnitude) : fallback;
}
}

Projection::Projection(Viewer viewer) {
    RenderCamera camera = viewer.camera();
    if (viewer && !camera)
        why = "the viewer has no camera";
    if (!camera)
        return;
    why = "the camera is parallel";
    if (camera.parallel())
        return;
    right = normalized(viewer.row(0));
    forward = normalized(viewer.row(1));
    up = normalized(viewer.row(2));
    position = viewer.row(3);
    offsetX = camera.viewOffset()[0];
    offsetY = camera.viewOffset()[1];
    windowX = camera.viewWindow()[0];
    windowY = camera.viewWindow()[1];
    float nearPlane = camera.nearPlane();
    nearZ = nearPlane > 0 ? nearPlane : 0.1f;
    const Viewport& viewport = camera.viewport();
    left = float(viewport.left);
    top = float(viewport.top);
    width = float(viewport.width);
    height = float(viewport.height);
    ok = windowX > 1e-5f && windowY > 1e-5f && width > 1 && height > 1 && finite(position);
    why = ok ? "" : "the view window or viewport is empty";
}

Vector Projection::toCamera(const Vector& point) const {
    Vector offset = point - position;
    return {dot(offset, right), dot(offset, forward), dot(offset, up)};
}

Projection::ScreenPoint Projection::toScreen(const Vector& cameraPoint) const {
    float normalizedX = (cameraPoint.x / cameraPoint.y - offsetX) / windowX;
    float normalizedY = (cameraPoint.z / cameraPoint.y - offsetY) / windowY;
    return {left + (normalizedX * 0.5f + 0.5f) * width, top + (0.5f - normalizedY * 0.5f) * height};
}

Vector Projection::through(float across, float down) const {
    float sideways = (across * 2.0f - 1.0f) * windowX + offsetX;
    float upward = (1.0f - down * 2.0f) * windowY + offsetY;
    return normalized(right * sideways + forward + up * upward);
}

std::optional<Projection::ScreenPoint> Projection::project(const Vector& point) const {
    if (!ok)
        return std::nullopt;
    const Vector cameraPoint = toCamera(point);
    if (!(cameraPoint.y > nearZ))
        return std::nullopt;
    const ScreenPoint screen = toScreen(cameraPoint);
    if (!std::isfinite(screen.x) || !std::isfinite(screen.y))
        return std::nullopt;
    return screen;
}

float changeEnergy(Ship ship, float delta) {
    return space::changeEnergy(ship.raw(), delta);
}

Vector ground(const Vector& point) {
    char* model = application::planetModel();
    Vector out = point;
    if (model)
        planet::toSurface(model, &out, &point);
    return finite(out) ? out : point;
}

std::optional<Vector> surfaceUnder(const Vector& point) {
    char* model = application::planetModel();
    if (!model)
        return std::nullopt;
    Vector out = point;
    planet::toSurface(model, &out, &point);
    return finite(out) ? std::optional(out) : std::nullopt;
}

bool findObstacles(const Vector& at, float radius, uint32_t kinds, ObstacleList& list) {
    char* manager = application::obstacleManager();
    if (!manager || !field<char*>(manager, 0x55a4) || !application::planetModel())
        return false;
    planet::nearbyObstacles(manager, &at, radius, kinds, &list);
    return true;
}

float groundClearance() {
    char* tuning = *space::shipTuning;
    return tuning ? field<float>(tuning, 0xc) : 0.0f;
}

bool inWater(const Vector& at) {
    char* model = application::planetModel();
    return model && planet::inWater(model, &at);
}

Terrain planetTerrain() {
    PlanetModel model(application::planetModel());
    Terrain terrain = model ? model.ground() : Terrain();
    if (!terrain || !readable(virtualTable(terrain.raw()) + 4, 4) ||
        virtualTable(terrain.raw())[4] != planet::terrainStateManager.at())
        return Terrain();
    return terrain;
}

TerrainState activeTerrainState() {
    Terrain terrain = planetTerrain();
    return terrain ? terrain.state() : TerrainState();
}

PropertyList terrainProperties() {
    Terrain terrain = planetTerrain();
    if (!terrain || !readable(virtualTable(terrain.raw()) + 2, 4) ||
        virtualTable(terrain.raw())[2] != planet::terrainProperties.at())
        return PropertyList();
    PropertyList properties = terrain.properties();
    return properties.readable(0x14) ? properties : PropertyList();
}

Image atmosphereImage(map::identifiers::Atmosphere type) {
    int index = int(type);
    return index >= 0 && index < 16 ? Image(planet::atmosphereImages[index]) : Image();
}

bool hasPlanetObject(PlanetRecord record, uint32_t tool) {
    return record && planet::hasPlanetObject(record.raw(), tool);
}

void placeToolObject(uint32_t tool, const Vector& at, float health) {
    planet::placeToolObject(tool, &at, health);
}

void markOnMinimap(void* minimap, GameData object, std::span<const float, 3> color) {
    struct Mark {
        uint32_t color = 0;
        Vector at{};
        uint32_t flags = 0x40000;
        void* object = nullptr;
        float size = 0.0f;
        std::array<uint32_t, 2> reserved{};
    };
    static_assert(sizeof(Mark) == 0x24);
    const Vector* at = object.body().position();
    if (!minimap || !object || !at)
        return;
    const std::array<float, 4> rgba{color[0], color[1], color[2], 1.0f};
    Mark mark{.color = planet::packColor(rgba.data()), .at = *at, .object = object.raw()};
    object.addReference();
    planet::unmarkOnMinimap(minimap, object.raw(), 1, 0);
    planet::markOnMinimap(minimap, object.raw(), &mark, 0, 0);
    object.release();
}

void showColonyMood(City city, float value, const Vector& at) {
    if (city && std::isfinite(value) && value != 0.0f)
        planet::colonyMood(city.raw(), value, &at);
}

void paintToolView(void* view, std::span<const float, 3> color) {
    char* model = view ? callVirtual<char*>(view, 0x2c) : nullptr;
    if (!model)
        return;
    for (size_t channel = 0; channel < color.size(); ++channel)
        field<float>(model, uint32_t(0x4c + channel * 4)) = color[channel];
}

bool playerSide(void* combatant) {
    GameData owner = combatant ? Combatant(combatant).owner() : GameData();
    if (!owner)
        return false;
    if (owner.politicalId() == playerPoliticalId())
        return true;
    Ship ship(owner.cast(map::identifiers::Type::Ship));
    return ship && ship.kind() == map::identifiers::ShipKind::Escort;
}

float* universeValue(uint32_t identifier) {
    constexpr uint16_t floatKind = 0xd;
    char* universe = *space::simulatorUniverse;
    void* list = universe ? field<void*>(universe, 0x10) : nullptr;
    void* property = nullptr;
    if (!list || !callVirtual<bool>(list, 0x24, identifier, &property) || !property ||
        field<uint16_t>(property, 0x12) != floatKind)
        return nullptr;
    return application::propertyFloat(property);
}

float standing(uint32_t who, uint32_t towards) {
    Relationship data = relationData(who, towards);
    return data && std::isfinite(data.standing()) ? data.standing() : 0.0f;
}

int relationLevel(uint32_t who, uint32_t towards) {
    void* relationships = civilization::relations();
    if (!relationships)
        return neutralRelation;
    return civilization::relationshipLevel(relationships,
                                           civilization::relationshipValue(relationships, who, towards, 0));
}

bool atWar(uint32_t first, uint32_t second) {
    void* relationships = civilization::relations();
    return relationships && civilization::isAtWar(relationships, first, second);
}

bool allied(uint32_t first, uint32_t second) {
    void* relationships = civilization::relations();
    return relationships && civilization::isAllied(relationships, first, second);
}

float apply(uint32_t who, uint32_t cause, map::identifiers::Relation event, float scale) {
    void* relationships = civilization::relations();
    return relationships ? civilization::applyRelationship(relationships, who, cause, uint32_t(event), scale) : 0.0f;
}

void declareWar(uint32_t enemy, uint32_t player) {
    apply(enemy, player, map::identifiers::Relation::DeclaredWar, 1.0f);
    apply(player, enemy, map::identifiers::Relation::DeclaredWar, 1.0f);
    for (Relationship data : {relationData(enemy, player), relationData(player, enemy)})
        if (data)
            data.flags() = (data.flags() | 1u) & ~2u;
}

void declarePeace(uint32_t enemy, uint32_t player) {
    for (Relationship data : {relationData(enemy, player), relationData(player, enemy)})
        if (data)
            data.flags() &= ~1u;
}

void showMood(City city, float value, const Vector* where) {
    Civilization owner = city ? city.civilization() : Civilization();
    void* floater = owner ? owner.floater() : nullptr;
    if (!floater)
        return;
    Vector at = where ? *where : city.position();
    civilization::moodFloater(floater, value, &at);
}

const Key* cultureKey(char* culture, uint32_t slot) {
    return civilization::cultureKey(culture, slot);
}

Key* findCreation(char* culture, uint32_t slot) {
    char* node = nullptr;
    civilization::cultureFind(culture, &node, &slot);
    return node && node != culture + 4 ? reinterpret_cast<Key*>(node + 0x14) : nullptr;
}

void culturePick(char* culture, uint32_t slot) {
    civilization::culturePick(culture, slot);
}

void addOrder(Vehicle vehicle, void* target, map::identifiers::Order kind, int flag) {
    civilization::addOrder(vehicle.raw(), target, int(kind), flag);
}

void addOrderAt(Vehicle vehicle, const Vector& at, int flag) {
    civilization::addOrderAt(vehicle.raw(), at.x, at.y, at.z, flag);
}

void clearOrders(Vehicle vehicle) {
    civilization::clearOrders(vehicle.raw());
}

void startOrders(Vehicle vehicle) {
    civilization::startOrders(vehicle.raw(), 0);
}

bool retargetBelief(Belief shell, const Vector& from, const Vector& to) {
    BeliefPath path = shell.path();
    if (!path || !path.readable(BeliefPath::byteSize))
        return false;
    std::span<Vector> points = path.points();
    int last = int(points.size()) - 1, current = path.segment();
    if (last < 1 || current < 0 || current >= last || !readable(points.data(), points.size_bytes()))
        return false;
    for (int i = current + 1; i <= last; ++i)
        points[i] = from + (to - from) * (float(i - current) / float(last - current));
    shell.aim() = to;
    return true;
}

bool superweaponCooling(Civilization owner, int ability) {
    return civilization::superweaponCooling(owner.raw(), ability);
}

void citySuperweapon(City city, int ability, const Vector& at) {
    civilization::citySuperweapon(city.raw(), ability, &at);
}

void startTimer(Timer timer) {
    civilization::timerStart(timer.raw());
}

void stopTimer(Timer timer) {
    if (timer)
        space::stopTimer(timer.raw());
}

int empireLevel(Empire empire) {
    return empire ? space::empireLevel(empire.raw()) : -1;
}

void* loadTool(uint32_t identifier) {
    void* manager = space::toolManager();
    void* tool = nullptr;
    const Key key{.instance = identifier, .type = 0, .group = 0};
    if (manager)
        space::loadTool(manager, &key, &tool);
    if (tool)
        Tool(tool).ready() = true;
    return tool;
}

bool fireTool(void* tool, GameData source, GameData target) {
    void* manager = space::toolManager();
    if (!manager || !tool || !source || !target)
        return false;
    Vector at = target.position();
    return space::fireTool(manager, tool, source.body().raw(), &at, -1, 0, target.body().raw(), 1);
}

bool placeEffect(VisualEffect effect, void* spatial, float scale) {
    if (!effect || !spatial)
        return false;
    effects::Catcher catcher;
    void* pointer = &catcher;
    application::placeEffectOver(spatial, &pointer);
    if (!catcher.kept)
        return false;
    catcher.caught.scale *= scale;
    effect.setSourceTransform(&catcher.caught);
    return true;
}

void loadSuperweaponTools(Civilization owner) {
    civilization::loadSuperweaponTools(owner.raw());
}

uint32_t showEvent(map::identifiers::Popup event, const wchar_t* text, EventClickFunction click,
                   EventRemovedFunction removed, float fadeSeconds, float removeSeconds) {
    char* log = *userInterface::eventLog;
    if (!log)
        return 0;
    uint32_t identifier =
        userInterface::showEvent(log, uint32_t(event), uint32_t(map::identifiers::Popup::Group), 0, nullptr, false, 0);
    EventItem item(identifier ? userInterface::findEvent(log, identifier) : nullptr);
    if (!item)
        return 0;
    item.user() = nullptr;
    item.click() = click;
    item.removed() = removed;
    item.fade() = fadeSeconds;
    item.removeAfter() = removeSeconds;
    userInterface::setEventText(log, identifier, text);
    return identifier;
}

void hideEvent(uint32_t event) {
    char* log = *userInterface::eventLog;
    if (log && event)
        userInterface::hideEvent(log, event, false);
}

void playAudio(uint32_t identifier) {
    application::playAudio(identifier, application::newAudioTrack());
}

int newAudioTrack() {
    return application::newAudioTrack();
}

void playAudio(uint32_t identifier, int track) {
    application::playAudio(identifier, track);
}

void stopAudio(int track) {
    application::stopAudio(track, 0);
}

int prepareAudio(uint32_t identifier) {
    static constexpr std::array<float, 3> origin{};
    int track = application::newAudioTrack();
    if (track)
        application::prepareAudio(identifier, track, origin.data());
    return track;
}

void setAudioProperty(int track, map::identifiers::AudioProperty property, float value) {
    application::setAudioProperty(track, uint32_t(property), value, 0);
}

void startAudio(int track) {
    application::startAudio(track);
}

Window newOverlay(const Rectangle& area) {
    char* object = userInterface::windowNew(Window::objectByteSize, 4, "UI/Window", userInterface::windowAllocator());
    if (!object)
        return Window();
    userInterface::windowConstructor(object);
    Window window(object + Window::faceOffset);
    window.addReference();
    window.setArea(area);
    window.setFill(0);
    window.setFlag(Window::visibleFlag, true);
    window.setFlag(Window::ignoresMouseFlag, true);
    return window;
}

void* loadPicture(const Key& picture) {
    void* image = nullptr;
    userInterface::loadImage(&picture, &image, false, -1, -1);
    return image;
}

void releasePicture(void* image) {
    if (image)
        callVirtual<int>(image, 0x4);
}

bool conversing() {
    char* manager = userInterface::commManager();
    return manager && userInterface::commScreenActive(manager);
}

Layout newLayout(const char* name) {
    constexpr uint32_t layoutTable = 0x01418360;
    char* memory = application::allocate(Layout::byteSize, name, 0, 0, nullptr, 0);
    char* made = memory ? userInterface::layoutConstructor(memory) : nullptr;
    if (!made || gameAddress(virtualTable(made)) != layoutTable)
        return Layout();
    Layout layout(made);
    layout.addReference();
    return layout;
}

StandardDrawable newPictureLook(void* image) {
    static constexpr std::array<uint32_t, 3> tables{0x01445238, 0x01445220, 0x014451e0};
    char* memory = image ? userInterface::windowNew(StandardDrawable::byteSize, 4, "UI/StdDrawable",
                                                    userInterface::windowAllocator())
                         : nullptr;
    char* made = memory ? userInterface::standardDrawableConstructor(memory) : nullptr;
    auto tableOf = [](const char* object) { return gameAddress(virtualTable(object)); };
    if (!made || std::array{tableOf(made), tableOf(made + StandardDrawable::layoutElementOffset),
                            tableOf(made + StandardDrawable::standardOffset)} != tables)
        return StandardDrawable();
    StandardDrawable look(made);
    look.addReference();
    look.setImage(0, InterfaceImage(image));
    look.setScale(StandardDrawable::Scale::Stretch);
    return look;
}
}

