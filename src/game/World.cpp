
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>

module beatuy.game.world;

namespace game {
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

float standing(uint32_t who, uint32_t towards) {
    Relationship data = relationData(who, towards);
    return data && std::isfinite(data.standing()) ? data.standing() : 0.0f;
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
}

