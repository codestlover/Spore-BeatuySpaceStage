
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

export module beatuy.game.world;

import beatuy.game.map;
import beatuy.game.math;
import beatuy.game.memory;
import beatuy.game.objects;

export namespace game {
template <ObjectView V> Array<V*> each(const map::Noun& kind) {
    void* nouns = map::application::nounManager();
    char* list = nouns ? map::application::nounList(nouns, map::application::nounThunkFirst.at(),
                                                    map::application::nounThunkSecond.at(), address(kind.thunk),
                                                    map::application::nounThunkFourth.at(), kind.noun)
                       : nullptr;
    return list ? Array<V*>(field<char*>(list, 4), field<char*>(list, 8)) : Array<V*>();
}

template <std::derived_from<WindowEffect> Effect> Effect newEffect(const char* name, const auto& constructor) {
    void* allocator = map::userInterface::windowAllocator();
    char* memory = map::userInterface::windowNew(Effect::byteSize, WindowEffect::alignment, name, allocator);
    if (!memory)
        return Effect();
    Effect effect(constructor(memory));
    effect.addReference();
    return effect;
}

bool inSpace();
map::identifiers::Context context();
bool onPlanet();
void markLeaving();
bool paused();

PlayerShipManager shipManager();
Ship playerShip();
Vector shipPosition();
uint32_t playerPoliticalId();
Empire playerEmpire();
Empire empireOf(uint32_t politicalId);
bool groxEmpire(uint32_t politicalId);
Empire findGrox();
inline constexpr size_t identityColors = 13;
size_t identityColor(uint32_t politicalId);
std::optional<int> addPlayerMoney(int amount);

Planet activePlanet();
PlanetRecord activeRecord();
map::identifiers::Technology planetTechnology();
Empire planetOwner();
uint32_t planetOwnerPoliticalId();
struct Capture {
    float percent;
    uint32_t capturer;
};
Capture planetCapture();
StarRecord activeStarRecord();
View hoveredStar();
GameData hoveredObject();
bool canEnterPlanet();
bool canEnterSystem();
float minimumZoom(map::identifiers::Context level);
float maximumZoom(map::identifiers::Context level);
Camera spaceCamera();
Camera planetCamera();

Viewer activeViewer();

class Projection {
public:
    explicit Projection(Viewer viewer);

    struct ScreenPoint {
        float x, y;
    };

    bool usable() const {
        return ok;
    }
    const char* failure() const {
        return why;
    }
    Vector eye() const {
        return position;
    }
    float nearDepth() const {
        return nearZ;
    }
    float viewportWidth() const {
        return width;
    }
    float viewportHeight() const {
        return height;
    }
    Vector toCamera(const Vector& point) const;
    ScreenPoint toScreen(const Vector& cameraPoint) const;
    std::optional<ScreenPoint> project(const Vector& point) const;
    Vector through(float across, float down) const;

private:
    Vector position{}, right{}, forward{}, up{};
    float windowX = 1, windowY = 1, offsetX = 0, offsetY = 0, nearZ = 0.1f;
    float left = 0, top = 0, width = 1, height = 1;
    bool ok = false;
    const char* why = "no viewer";
};
float changeEnergy(Ship ship, float delta);

Vector ground(const Vector& point);
std::optional<Vector> surfaceUnder(const Vector& point);
bool findObstacles(const Vector& at, float radius, uint32_t kinds, ObstacleList& list);
float groundClearance();
bool inWater(const Vector& at);
Terrain planetTerrain();
TerrainState activeTerrainState();
PropertyList terrainProperties();
Image atmosphereImage(map::identifiers::Atmosphere type);

bool hasPlanetObject(PlanetRecord record, uint32_t tool);
void placeToolObject(uint32_t tool, const Vector& at, float health);
void markOnMinimap(void* minimap, GameData object, std::span<const float, 3> color);
void paintToolView(void* view, std::span<const float, 3> color);
void showColonyMood(City city, float value, const Vector& at);

bool playerSide(void* combatant);
float* universeValue(uint32_t identifier);
float standing(uint32_t who, uint32_t towards);
inline constexpr int worstRelation = 0, neutralRelation = 2;
int relationLevel(uint32_t who, uint32_t towards);
bool atWar(uint32_t first, uint32_t second);
bool allied(uint32_t first, uint32_t second);
float apply(uint32_t who, uint32_t cause, map::identifiers::Relation event, float scale);
void declareWar(uint32_t enemy, uint32_t player);
void declarePeace(uint32_t enemy, uint32_t player);
void startWar(Empire attacker, Empire defender);
void showMood(City city, float value, const Vector* where = nullptr);

const Key* cultureKey(char* culture, uint32_t slot);
Key* findCreation(char* culture, uint32_t slot);
void culturePick(char* culture, uint32_t slot);

void addOrder(Vehicle vehicle, void* target, map::identifiers::Order kind, int flag);
void addOrderAt(Vehicle vehicle, const Vector& at, int flag);
void clearOrders(Vehicle vehicle);
void startOrders(Vehicle vehicle);

bool retargetBelief(Belief shell, const Vector& from, const Vector& to);

bool superweaponCooling(Civilization owner, int ability);
void citySuperweapon(City city, int ability, const Vector& at);
void startTimer(Timer timer);
void stopTimer(Timer timer);
int empireLevel(Empire empire);
Array<PlanetRecord*> planetsOf(StarRecord star);
bool eventOnPlanet(PlanetRecord planet);
RaidEvent startRaid(map::identifiers::Mission mission, PlanetRecord planet, Empire owner, uint32_t attacker,
                    int bombers, bool logged);
void claimStar(StarRecord star, Empire empire);
void* loadTool(uint32_t identifier);
bool fireTool(void* tool, GameData source, GameData target);
bool placeEffect(VisualEffect effect, void* spatial, float scale);
void destroy(GameData object);
void placeEffectAt(VisualEffect effect, const Vector& at, const Vector& up, float scale);
void loadSuperweaponTools(Civilization owner);

uint32_t showEvent(map::identifiers::Popup event, const wchar_t* text, EventClickFunction click,
                   EventRemovedFunction removed, float fadeSeconds, float removeSeconds);
void hideEvent(uint32_t event);
void playAudio(uint32_t identifier);
int newAudioTrack();
void playAudio(uint32_t identifier, int track);
void stopAudio(int track);
int prepareAudio(uint32_t identifier);
void setAudioProperty(int track, map::identifiers::AudioProperty property, float value);
void startAudio(int track);
Window newOverlay(const Rectangle& area);
void* loadPicture(const Key& picture);
void releasePicture(void* image);
bool conversing();
Layout newLayout(const char* name);
StandardDrawable newPictureLook(void* image);
}

