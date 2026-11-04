
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

#include <Windows.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <optional>

import beatuy.core.hooks;
import beatuy.core.language;
import beatuy.core.mod;
import beatuy.core.name;
import beatuy.game;

using InitializationFunction = void (*)();
using AddPostInitializationFunction = void (*)(InitializationFunction);
constexpr wchar_t launcherCore[] = L"SporeModAPI.dll";
constexpr char addPostInitializationName[] = "?AddPostInitFunction@ModAPI@@YAXP6AXXZ@Z";

void* reinforceOriginal = nullptr;

namespace {
namespace application = map::application;
namespace space = map::space;
namespace planet = map::planet;
namespace civilization = map::civilization;
namespace tribe = map::tribe;
namespace saving = map::saving;
using map::identifiers::Context;

Mod& mod() {
    return Mod::get();
}

bool __fastcall toolTargetHook(void* tool, void*, void* target) {
    if (mod().escort.blocks(target))
        return false;
    return space::toolTargetCheck.callOriginal(tool, target);
}

int __fastcall takeDamageHook(void* combatant, void*, float damage, uint32_t politicalId, int type, const void* at,
                              void* attacker) {
    bool planet = game::onPlanet();
    if (planet) {
        if (mod().civilization.blockHeal(combatant, politicalId, type))
            return 0;
        damage = mod().civilization.superHit(combatant, attacker, damage, type);
        mod().civilization.onDamage(combatant, politicalId, attacker, damage, type);
        type = mod().civilization.damageType(combatant, attacker, type);
    }
    const game::ToolObject struck = planet ? mod().generator.alienTarget(combatant, politicalId) : game::ToolObject();
    const bool standing = struck && struck.fighter().health() > 0.0f;
    const int result = space::combatantTakeDamage.callOriginal(combatant, damage, politicalId, type, at, attacker);
    if (struck)
        mod().generator.answerStrike(struck, standing);
    return result;
}

void __fastcall leavePlanetHook(void* transition, void*, int argument) {
    if (game::inSpace() && game::context() == Context::Planet)
        mod().leavePlanet(false);
    space::leavePlanet.callOriginal(transition, argument);
}

void __fastcall destroyedHook(void* ship, void*) {
    if (ship == game::playerShip().raw() && game::context() == Context::Planet)
        mod().leavePlanet(true);
    space::playerShipDestroyed.callOriginal(ship);
}

void __fastcall exitHook(void* mode, void*) {
    if (game::context() == Context::Planet)
        mod().leavePlanet(false);
    mod().picker.close();
    mod().orders.close();
    space::modeExit.callOriginal(mode);
}

float __fastcall applyHook(void* relations, void*, uint32_t who, uint32_t cause, uint32_t event, float scale) {
    if (!mod().civilization.ownCall())
        game::onPlanet();
    return civilization::applyRelationship.callOriginal(relations, who, cause, event, scale);
}

void __fastcall addOrderHook(void* vehicle, void*, void* target, int kind, int flag) {
    if (mod().civilization.onOrder(vehicle))
        civilization::addOrder.callOriginal(vehicle, target, kind, flag);
}

void __fastcall addOrderAtHook(void* vehicle, void*, float x, float y, float z, int flag) {
    if (mod().civilization.onOrder(vehicle))
        civilization::addOrderAt.callOriginal(vehicle, x, y, z, flag);
}

void* __fastcall getWeaponHook(void* combatant, void*) {
    if (mod().civilization.playerSide(game::Combatant(combatant).target())) {
        game::Vehicle vehicle = game::Vehicle::ofFighter(combatant);
        if (vehicle.airWeapon() && vehicle.purpose() == map::identifiers::Purpose::Military &&
            vehicle.locomotion() != map::identifiers::Locomotion::Air)
            return vehicle.airWeapon();
    }
    return civilization::vehicleGetWeapon.callOriginal(combatant);
}

bool __fastcall inRangeHook(void* combatant, void*, void* other) {
    if (other) {
        int reach = mod().civilization.inReach(combatant, other);
        if (reach >= 0)
            return reach > 0;
    }
    return civilization::vehicleInRange.callOriginal(combatant, other);
}

void __fastcall setTargetHook(void* combatant, void*, void* target) {
    if (!mod().civilization.ownCall()) {
        void* prey = mod().civilization.lockedTarget(game::Vehicle::ofFighter(combatant).raw());
        if (prey)
            target = prey;
    }
    civilization::vehicleSetTarget.callOriginal(combatant, target);
}

int __fastcall vehicleDamageHook(void* combatant, void*, float damage, uint32_t politicalId, int type, const void* at,
                                 void* attacker) {
    void* outer = mod().radio.speaker;
    mod().radio.speaker = game::Vehicle::ofFighter(combatant).raw();
    bool planet = game::onPlanet();
    if (planet)
        politicalId = mod().civilization.healerFor(combatant, politicalId, type);
    int result = civilization::vehicleTakeDamage.callOriginal(combatant, damage, politicalId, type, at, attacker);
    mod().radio.speaker = outer;
    return result;
}

void __fastcall voiceQueueHook(void* queue, void*, uint32_t voice) {
    if (mod().radio.speaker)
        mod().radio.onQueued(voice);
    civilization::voiceQueue.callOriginal(queue, voice);
}

void __fastcall atmosphereHook(char* state, void*, float altitude, int force) {
    planet::atmosphereColor.callOriginal(state, altitude, force);
    mod().glow.onAtmosphere(state, altitude);
}

void __fastcall atmosphereModelHook(char* state, void*, float* model, int force) {
    planet::atmosphereModel.callOriginal(state, model, force);
    mod().glow.onLighting(state, model);
}

void __fastcall bakeLayerHook(void* layer, void*, uint32_t secondArgument, uint32_t thirdArgument,
                              uint32_t fourthArgument, uint32_t fifthArgument) {
    mod().glow.beginBake();
    planet::bakeLayer.callOriginal(layer, secondArgument, thirdArgument, fourthArgument, fifthArgument);
    mod().glow.endBake();
}

bool __fastcall systemWheelHook(void* camera, void*, int delta, float x, float y, int state) {
    return mod().quickExit.onWheel(delta) || space::systemCameraWheel.callOriginal(camera, delta, x, y, state);
}

bool __fastcall planetWheelHook(void* camera, void*, int delta, float x, float y, int state) {
    return mod().quickExit.onWheel(delta) || space::planetCameraWheel.callOriginal(camera, delta, x, y, state);
}

bool __fastcall beliefUpdateHook(void* projectile, void*) {
    if (!game::onPlanet())
        return civilization::beliefUpdate.callOriginal(projectile);
    mod().civilization.beginBelief(game::Belief::ofProjectile(projectile).raw());
    bool alive = civilization::beliefUpdate.callOriginal(projectile);
    mod().civilization.endBelief();
    return alive;
}

bool __fastcall fireToolHook(void* manager, void*, void* tool, void* source, const game::Vector* at, int fifthArgument,
                             int sixthArgument, void* target, int flag) {
    if (game::onPlanet() && mod().civilization.blockShot(tool, source, target))
        return false;
    return space::fireTool.callOriginal(manager, tool, source, at, fifthArgument, sixthArgument, target, flag);
}

void* __fastcall spawnVehicleHook(void* city, void*, int purpose, int locomotion, game::Key key, int inSpace) {
    if (inSpace & 0xff && game::onPlanet())
        mod().civilization.vehicleDesign(city, purpose, locomotion, key);
    return civilization::spawnVehicle.callOriginal(city, purpose, locomotion, key, inSpace);
}

void __fastcall launchHook(void* shell, void*, void* owner, void* vehicle, void* tool, void* target, const void* at,
                           float speed, int arc, int spin) {
    civilization::beliefLaunch.callOriginal(shell, owner, vehicle, tool, target, at, speed, arc, spin);
    mod().civilization.onBeliefLaunch(static_cast<char*>(shell));
}

bool __fastcall shipInRangeHook(void* combatant, void*, void* other) {
    if (other && game::onPlanet() && mod().civilization.spared(other))
        return false;
    return space::shipInRange.callOriginal(combatant, other);
}

int __fastcall shipDamageHook(void* combatant, void*, float damage, uint32_t politicalId, int type, const void* at,
                              void* attacker) {
    game::Vector push;
    if (game::onPlanet())
        at = mod().civilization.push(combatant, attacker, static_cast<const game::Vector*>(at), push);
    return space::shipTakeDamage.callOriginal(combatant, damage, politicalId, type, at, attacker);
}

int __fastcall repairFireHook(void* strategy, void*, void* tool, const void* aim, int flag) {
    mod().escort.maskFormer();
    int result = space::repairAreaFire.callOriginal(strategy, tool, aim, flag);
    mod().escort.unmask();
    return result;
}

bool __fastcall spearUpdateHook(void* projectile, void*) {
    int verdict = mod().tribe.spearTick(projectile);
    if (verdict >= 0)
        return verdict != 0;
    return mod().tribe.spearAfter(projectile, tribe::spearUpdate.callOriginal(projectile));
}

int __cdecl panicTickHook(void* creature, uint32_t secondArgument, uint32_t thirdArgument, uint32_t fourthArgument,
                          uint32_t fifthArgument, uint32_t nodeState, uint32_t seventhArgument, uint32_t eighthArgument,
                          uint32_t ninthArgument, uint32_t tenthArgument) {
    if (mod().tribe.holds(creature))
        return 1;
    return tribe::panicTick.callOriginal(creature, secondArgument, thirdArgument, fourthArgument, fifthArgument,
                                         nodeState, seventhArgument, eighthArgument, ninthArgument, tenthArgument);
}

int __cdecl cowerTickHook(void* creature, uint32_t secondArgument, uint32_t thirdArgument, uint32_t fourthArgument,
                          uint32_t fifthArgument, uint32_t nodeState, uint32_t seventhArgument, uint32_t eighthArgument,
                          uint32_t ninthArgument, uint32_t tenthArgument) {
    if (mod().tribe.holds(creature))
        return 1;
    return tribe::cowerTick.callOriginal(creature, secondArgument, thirdArgument, fourthArgument, fifthArgument,
                                         nodeState, seventhArgument, eighthArgument, ninthArgument, tenthArgument);
}

const char16_t* __fastcall localeTextHook(void* table, void*, uint32_t identifier) {
    if (const char16_t* text = feature::Generator::localeText(game::field<uint32_t>(table, 8), identifier))
        return text;
    return application::localeTableText.callOriginal(table, identifier);
}

void __cdecl markToolObjectHook(void* object, void* minimap) {
    if (const auto color = object && minimap ? mod().generator.alienColor(game::GameData(object)) : std::nullopt) {
        game::GameData(object).setPoliticalId(game::planetOwnerPoliticalId());
        game::markOnMinimap(minimap, game::GameData(object), *color);
        return;
    }
    planet::markToolObject.callOriginal(object, minimap);
}

void* __fastcall toolObjectViewHook(void* prototype, void*, void* object) {
    void* view = planet::toolObjectView.callOriginal(prototype, object);
    if (const auto color = view && object ? mod().generator.alienColor(game::GameData(object)) : std::nullopt)
        game::paintToolView(view, *color);
    return view;
}

void* __cdecl rolloverEmpire(void* object) {
    if (object && mod().generator.alienColor(game::GameData(object)))
        if (const game::Empire owner = game::planetOwner())
            return owner.raw();
    return space::playerEmpire();
}

__declspec(naked) void rolloverEmpireHook() {
    __asm {
        push edi
        call rolloverEmpire
        add esp, 4
        ret
    }
}

bool redirectRolloverEmpire() {
    const uintptr_t next = reinterpret_cast<uintptr_t>(planet::rolloverEmpireCall.at()) + 6;
    const uint32_t relative = uint32_t(reinterpret_cast<uintptr_t>(&rolloverEmpireHook) - next);
    return hooks::patch(planet::rolloverEmpireCall, 2, std::bit_cast<std::array<uint8_t, 4>>(relative));
}

bool __fastcall placeObjectUpdateHook(void* strategy, void*, char* tool, bool show, const char16_t** failText) {
    if (const std::optional<bool> verdict = feature::Generator::placement(strategy, tool, show, failText))
        return *verdict;
    return space::placeObjectUpdate.callOriginal(strategy, tool, show, failText);
}

void __fastcall shipSetTargetHook(void* combatant, void*, void* target) {
    space::shipSetTarget.callOriginal(combatant, mod().orders.target(combatant, target));
}

void __fastcall shipSetTargetInBandHook(void* ship, void*, void* target, int band) {
    void* combatant = game::Ship(ship).fighter().raw();
    if (const std::optional<void*> ordered = mod().orders.orderedTarget(combatant)) {
        space::shipSetTarget.callOriginal(combatant, *ordered);
        return;
    }
    space::shipSetTargetInBand.callOriginal(ship, target, band);
}

void __fastcall shipAddHostileHook(void* combatant, void*, void* attacker) {
    space::shipAddHostile.callOriginal(combatant, attacker);
    mod().orders.onHostile(combatant, attacker);
}

void __fastcall spaceGameUpdateHook(void* simulator, void*, uint32_t elapsedLowerWord, uint32_t elapsedUpperWord) {
    space::spaceGameUpdate.callOriginal(simulator, elapsedLowerWord, elapsedUpperWord);
    mod().orders.onGameUpdate(elapsedLowerWord);
}

bool __fastcall playerShipWriteHook(void* serializable, void*, void* stream) {
    if (const std::optional<bool> written = mod().orders.write(serializable, stream))
        return *written;
    return saving::playerShipWrite.callOriginal(serializable, stream);
}

bool __fastcall playerShipReadHook(void* serializable, void*, void* stream) {
    if (const std::optional<bool> read = mod().orders.read(serializable, stream))
        return *read;
    return saving::playerShipRead.callOriginal(serializable, stream);
}

int __fastcall rallyCallHook(void* strategy, void*, void* tool, const void* aim, int flag) {
    const int result = space::rallyCallFire.callOriginal(strategy, tool, aim, flag);
    mod().orders.onRallyCall();
    return result;
}

void __cdecl onReinforce() {
    mod().glow.onSquad();
}

__declspec(naked) void reinforceHook() {
    __asm {
        pushad
        pushfd
        call onReinforce
        popfd
        popad
        jmp dword ptr [reinforceOriginal]
    }
}

bool __fastcall mouseDownHook(void* mode, void*, int button, float x, float y, int state) {
    mod().mouseDown(button);
    return space::modeMouseDown.callOriginal(mode, button, x, y, state);
}

void __fastcall updateHook(void* mode, void*, uint32_t secondArgument, uint32_t thirdArgument) {
    space::modeUpdate.callOriginal(mode, secondArgument, thirdArgument);
    mod().tick();
}

int installAll() {
    using hooks::install;
    const bool installed[] = {
        install(space::modeUpdate, &updateHook),
        install(space::modeExit, &exitHook),
        install(space::modeMouseDown, &mouseDownHook),
        install(planet::bakeLayer, &bakeLayerHook),
        install(space::systemCameraWheel, &systemWheelHook),
        install(space::planetCameraWheel, &planetWheelHook),
        install(civilization::beliefUpdate, &beliefUpdateHook),
        install(space::shipTakeDamage, &shipDamageHook),
        install(space::shipInRange, &shipInRangeHook),
        install(space::shipSetTarget, &shipSetTargetHook),
        install(space::shipSetTargetInBand, &shipSetTargetInBandHook),
        install(space::shipAddHostile, &shipAddHostileHook),
        install(space::spaceGameUpdate, &spaceGameUpdateHook),
        install(saving::playerShipWrite, &playerShipWriteHook),
        install(saving::playerShipRead, &playerShipReadHook),
        install(space::repairAreaFire, &repairFireHook),
        install(space::rallyCallFire, &rallyCallHook),
        install(tribe::spearUpdate, &spearUpdateHook),
        install(civilization::vehicleGetWeapon, &getWeaponHook),
        install(civilization::vehicleTakeDamage, &vehicleDamageHook),
        install(civilization::vehicleInRange, &inRangeHook),
        install(civilization::vehicleSetTarget, &setTargetHook),
        install(application::localeTableText, &localeTextHook),
        install(space::placeObjectUpdate, &placeObjectUpdateHook),
        install(planet::markToolObject, &markToolObjectHook),
        install(planet::toolObjectView, &toolObjectViewHook),
        redirectRolloverEmpire(),
        install(space::toolTargetCheck, &toolTargetHook),
        install(space::combatantTakeDamage, &takeDamageHook),
        install(space::leavePlanet, &leavePlanetHook),
        install(space::playerShipDestroyed, &destroyedHook),
        install(space::callReinforcements, &reinforceHook, &reinforceOriginal),
        install(civilization::applyRelationship, &applyHook),
        install(civilization::addOrder, &addOrderHook),
        install(civilization::addOrderAt, &addOrderAtHook),
        install(space::fireTool, &fireToolHook),
        install(civilization::spawnVehicle, &spawnVehicleHook),
        install(civilization::beliefLaunch, &launchHook),
        install(civilization::voiceQueue, &voiceQueueHook),
        install(tribe::panicTick, &panicTickHook),
        install(tribe::cowerTick, &cowerTickHook),
        install(planet::atmosphereColor, &atmosphereHook),
        install(planet::atmosphereModel, &atmosphereModelHook),
    };
    if (application::localeTableText.original)
        activeLanguage();
    return int(std::ranges::count(installed, false));
}

void initialize() {
    game::base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    bool known = map::hostMatches() && map::verify();
    if (known)
        mod().orders.prepare();
    int failed = known ? installAll() : 0;
    if (known && !failed)
        return;
    using beatuy::join, beatuy::wideName;
    static constexpr auto unsupported =
        join(wideName, L" could not start: this version of Spore is not supported.\n\n", wideName,
             L" supports only Spore Galactic Adventures with the March 2017 patch (SporeApp_ModAPIFix.exe).");
    static constexpr auto partly =
        join(wideName, L" started only partly: another mod has already changed some of the same game code.");
    MessageBoxW(nullptr, known ? partly.data() : unsupported.data(), wideName, MB_OK | MB_ICONERROR);
}
}

BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        HMODULE core = GetModuleHandleW(launcherCore);
        auto addPostInitialization =
            core ? reinterpret_cast<AddPostInitializationFunction>(GetProcAddress(core, addPostInitializationName))
                 : nullptr;
        if (!addPostInitialization)
            return FALSE;
        addPostInitialization(initialize);
    }
    return TRUE;
}

