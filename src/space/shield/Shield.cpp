
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>

module beatuy.feature.shield;

import beatuy.game;

namespace feature {
namespace {
using map::identifiers::Type;
using shield::settings;

auto used(auto& list, int count) {
    return std::span(list).first(size_t(count));
}

char* rolloverOf(game::GameData hovered) {
    char* pool = *map::userInterface::rolloverPool;
    uint32_t cursor = 0;
    for (char* entry = pool ? map::userInterface::rolloverNext(pool, &cursor) : nullptr; entry;
         entry = map::userInterface::rolloverNext(pool, &cursor))
        if (game::field<void*>(entry, 4) == hovered.raw() && game::field<void*>(entry, 0x50))
            return entry;
    return nullptr;
}

float cityStrengthOf(game::City city) {
    game::GameData hall(city.hall());
    void* fighter = hall ? hall.cast(Type::Combatant) : nullptr;
    auto buildings = city.buildings();
    bool listed = std::ranges::any_of(buildings, [&](game::GameData one) { return one.raw() == hall.raw(); });
    float count = float(std::ranges::distance(buildings)) + float(!listed);
    float strength =
        fighter ? std::floor(game::Combatant(fighter).maximumHealth() * count / settings.cityStrengthShare) : 0.0f;
    return std::isfinite(strength) && strength > 0.0f ? strength : settings.cityStrength;
}

float cityRadiusOf(game::City city) {
    game::Walls walls = city.walls();
    float outer = walls ? walls.outerRadius() : 0.0f;
    return (std::isfinite(outer) && outer > 1.0f && outer < 400.0f ? outer : settings.cityRadius) *
           settings.cityPadding;
}

std::optional<float> domeEntry(const game::Vector& from, const game::Vector& toward, game::City city) {
    const game::Vector* middle = city.place().position();
    if (!middle)
        return std::nullopt;
    float radius = cityRadiusOf(city);
    game::Vector offset = from - *middle, above = game::unit(*middle);
    float along = game::dot(offset, toward);
    float inside = along * along - game::dot(offset, offset) + radius * radius;
    if (inside < 0.0f)
        return std::nullopt;
    float reach = std::sqrt(inside);
    for (float distance : {-along - reach, reach - along})
        if (distance > 0.0f && game::dot(offset + toward * distance, above) >= 0.0f)
            return distance;
    return std::nullopt;
}

bool outside(const game::Vector& point, game::City city) {
    const game::Vector* middle = city.place().position();
    return middle && game::distance(point, *middle) > cityRadiusOf(city);
}
}

std::span<Shield::Barrier> Shield::barriers() {
    return used(barrierList, barrierCount);
}

Shield::Barrier* Shield::barrierOf(void* combatant) {
    for (Barrier& barrier : barriers()) {
        if (barrier.ship && barrier.ship.fighter().raw() == combatant)
            return &barrier;
        for (game::GameData part : used(barrier.parts, barrier.partCount))
            if (part.cast(Type::Combatant) == combatant)
                return &barrier;
    }
    return nullptr;
}

bool Shield::elitePlanet() const {
    if (!game::onPlanet() || game::planetTechnology() != map::identifiers::Technology::Empire)
        return false;
    game::Empire owner = game::planetOwner();
    return owner && owner.politicalId() != game::playerPoliticalId() &&
           game::empireLevel(owner) + 1 >= settings.eliteLevel;
}

float Shield::scaleOf(const Barrier& barrier) {
    return barrier.ship ? settings.shipScale : cityRadiusOf(barrier.city) / settings.domeRadius;
}

const shield::Look& Shield::lookOf(const Barrier& barrier) {
    return barrier.ship ? settings.bubbleLook : settings.domeLook;
}

void* Shield::spatialOf(const Barrier& barrier) {
    return barrier.ship ? barrier.ship.body().raw() : barrier.city.place().raw();
}

Shield::Barrier* Shield::raiseShip(game::Ship ship, DWORD now) {
    if (Barrier* known = barrierOf(ship.fighter().raw()))
        return known;
    if (!ship || barrierCount >= shield::maximumBarriers)
        return nullptr;
    float strength = ship.fighter().maximumHealth() * settings.shipStrength;
    if (!(strength > 0.0f))
        return nullptr;
    ship.addReference();
    Barrier& barrier = barrierList[barrierCount++];
    barrier = Barrier{.ship = ship, .strength = strength, .left = strength, .until = now + settings.shipMilliseconds};
    barrier.dome = game::EffectsManager::get().create(settings.bubble);
    if (barrier.dome) {
        game::placeEffect(barrier.dome, ship.body().raw(), 1.0f);
        barrier.dome.start();
    }
    return &barrier;
}

void Shield::play(Clip& clip, uint32_t sound, DWORD milliseconds) {
    if (!clip.track)
        clip.track = game::newAudioTrack();
    if (!clip.track)
        return;
    game::playAudio(sound, clip.track);
    clip.until = GetTickCount() + milliseconds;
    clip.heard = true;
}

void Shield::quiet(Clip& clip) {
    if (clip.heard)
        game::stopAudio(clip.track);
    clip.heard = false;
}

bool Shield::raiseCity(game::City city, DWORD now) {
    if (barrierCount >= shield::maximumBarriers)
        return false;
    Barrier& barrier = barrierList[barrierCount];
    float strength = cityStrengthOf(city);
    barrier = Barrier{.city = city, .strength = strength, .left = strength, .until = now + settings.cityMilliseconds};
    auto guard = [&barrier](game::GameData part) {
        void* combatant = part ? part.cast(Type::Combatant) : nullptr;
        if (!combatant || !part.alive() || barrier.partCount >= shield::maximumParts)
            return;
        part.addReference();
        barrier.parts[barrier.partCount++] = part;
    };
    guard(game::GameData(city.hall()));
    for (game::GameData building : city.buildings())
        guard(building);
    for (game::Turret turret : city.turrets())
        guard(turret);
    if (!barrier.partCount)
        return false;
    city.addReference();
    ++barrierCount;
    barrier.dome = game::EffectsManager::get().create(settings.dome);
    if (barrier.dome) {
        game::placeEffect(barrier.dome, city.place().raw(), scaleOf(barrier));
        barrier.dome.start();
    }
    return true;
}

void Shield::lower(int index, bool broken) {
    Barrier& barrier = barrierList[index];
    if (broken) {
        burst(barrier, lookOf(barrier).shatter, settings.burstMilliseconds);
        for (uint32_t sound : settings.shatterSounds)
            game::playAudio(sound);
    }
    if (barrier.ship) {
        auto captain = std::ranges::find(used(captains, captainCount), barrier.ship, &Captain::ship);
        if (captain != used(captains, captainCount).end())
            captain->shieldReady = GetTickCount() + settings.shipCooldownMilliseconds;
        barrier.ship.release();
    }
    for (game::GameData part : used(barrier.parts, barrier.partCount))
        part.release();
    if (barrier.dome && barrier.ship) {
        barrier.dome.stop();
        barrier.dome.release();
    }
    if (barrier.dome && !barrier.ship)
        end(barrier.dome);
    if (barrier.cracks) {
        barrier.cracks.halt();
        barrier.cracks.release();
    }
    if (barrier.city) {
        auto ward = std::ranges::find(used(wards, wardCount), barrier.city, &Ward::city);
        if (ward != used(wards, wardCount).end())
            ward->ready = GetTickCount() + settings.cityCooldownMilliseconds;
        barrier.city.release();
    }
    barrierList[index] = barrierList[--barrierCount];
    barrierList[barrierCount] = Barrier{};
}

void Shield::end(game::VisualEffect effect) {
    effect.stop();
    if (endedCount < shield::maximumBarriers) {
        ended[endedCount++] = Ended{.effect = effect, .at = GetTickCount() + settings.endMilliseconds};
        return;
    }
    effect.halt();
    effect.release();
}

void Shield::clearEnded(DWORD now) {
    for (int i = endedCount - 1; i >= 0; --i) {
        if (LONG(now - ended[i].at) < 0)
            continue;
        ended[i].effect.halt();
        ended[i].effect.release();
        ended[i] = ended[--endedCount];
        ended[endedCount] = Ended{};
    }
}

void Shield::crack(Barrier& barrier, bool loud) {
    size_t stage = size_t(barrier.stage++);
    for (uint32_t sound : std::span(settings.crackSounds[stage]).first(loud ? settings.crackSounds[stage].size() : 0))
        game::playAudio(sound);
    burst(barrier, lookOf(barrier).cracks[stage], settings.burstMilliseconds);
    if (barrier.cracks) {
        barrier.cracks.halt();
        barrier.cracks.release();
    }
    barrier.cracks = game::EffectsManager::get().create(lookOf(barrier).cracked[stage]);
    if (!barrier.cracks)
        return;
    game::placeEffect(barrier.cracks, spatialOf(barrier), scaleOf(barrier));
    barrier.cracks.start();
}

void Shield::burst(const Barrier& barrier, uint32_t identifier, DWORD milliseconds) {
    flash(barrier.ship ? game::GameData(barrier.ship) : game::GameData(barrier.city), spatialOf(barrier), identifier,
          scaleOf(barrier), milliseconds);
}

void Shield::flash(game::GameData holder, void* spatial, uint32_t identifier, float scale, DWORD milliseconds) {
    if (burstCount >= shield::maximumBursts)
        return;
    game::VisualEffect effect =
        holder && spatial ? game::EffectsManager::get().create(identifier) : game::VisualEffect();
    if (!effect)
        return;
    game::placeEffect(effect, spatial, scale);
    effect.start();
    holder.addReference();
    bursts[burstCount++] = Burst{
        .holder = holder, .spatial = spatial, .effect = effect, .scale = scale, .until = GetTickCount() + milliseconds};
}

void Shield::moveBursts(DWORD now) {
    for (int i = burstCount - 1; i >= 0; --i) {
        Burst& shown = bursts[i];
        if (LONG(now - shown.until) < 0 && shown.holder.alive()) {
            game::placeEffect(shown.effect, shown.spatial, shown.scale);
            continue;
        }
        shown.effect.stop();
        shown.effect.release();
        shown.holder.release();
        bursts[i] = bursts[--burstCount];
    }
}

Shield::Captain* Shield::captainOf(game::Ship ship) {
    auto found = std::ranges::find(used(captains, captainCount), ship, &Captain::ship);
    if (found != used(captains, captainCount).end())
        return &*found;
    if (captainCount >= shield::maximumCaptains)
        return nullptr;
    ship.addReference();
    DWORD stagger = DWORD(captainCount) * settings.missileMilliseconds / shield::maximumCaptains;
    captains[captainCount] = Captain{.ship = ship, .nextMissile = GetTickCount() + stagger, .shieldReady = 0};
    return &captains[captainCount++];
}

void Shield::fireMissile(Captain& captain, game::GameData target, DWORD now) {
    if (!target || !target.alive() || !target.cast(Type::Combatant) ||
        game::distance(captain.ship.position(), target.position()) > settings.missileRange)
        return;
    auto flying =
        std::ranges::count_if(used(flights, flightCount), [](const Flight& flight) { return bool(flight.missile); });
    if (flying >= settings.missilesInFlight)
        return;
    if (!missileTool)
        missileTool = game::loadTool(settings.missile);
    if (!missileTool)
        return;
    launcher = captain.ship;
    aimed = target;
    firing = true;
    bool fired = game::fireTool(missileTool, captain.ship, target);
    firing = false;
    captain.nextMissile = now + settings.missileMilliseconds;
    if (fired)
        play(launchClip, settings.launchSound, settings.launchSoundMilliseconds);
}

bool Shield::launching() const {
    return firing;
}

bool Shield::covers(void* city) const {
    return std::ranges::any_of(used(barrierList, barrierCount),
                               [city](const Barrier& barrier) { return barrier.city.raw() == city; });
}

bool Shield::onMissile(void* missile) {
    if (!firing)
        return false;
    if (!missile || flightCount >= shield::maximumMissiles)
        return true;
    game::Missile shot(missile);
    shot.addReference();
    aimed.addReference();
    Flight& flight = flights[flightCount++];
    flight = Flight{.missile = shot, .target = aimed, .seen = aimed.position(), .sampled = GetTickCount()};
    game::Guidance guidance = shot.guidance();
    if (guidance)
        guidance.flownMilliseconds() += settings.missileHeadStart;
    flight.trail = game::EffectsManager::get().create(settings.trails[game::identityColor(launcher.politicalId())]);
    if (!flight.trail)
        return true;
    game::placeEffect(flight.trail, shot.body().raw(), 1.0f);
    flight.trail.start();
    return true;
}

float Shield::agility(void* ship, float factor) const {
    auto leads = [ship](const Captain& captain) { return captain.ship.raw() == ship; };
    bool agile = std::ranges::any_of(std::span(captains).first(size_t(captainCount)), leads);
    return std::array{1.0f, factor}[size_t(agile)];
}

float Shield::speedOf(void* ship) const {
    return agility(ship, settings.agileSpeed);
}

float Shield::accelerationOf(void* ship) const {
    return agility(ship, settings.agileAcceleration);
}

void Shield::settle(Barrier& barrier, bool loud) {
    float share = barrier.left / barrier.strength;
    while (barrier.stage < int(settings.stages.size()) && share <= settings.stages[size_t(barrier.stage)])
        crack(barrier, loud);
}

bool Shield::jump(Captain& captain, const game::Vector& threat, float danger, float incoming) {
    DWORD now = GetTickCount();
    if (LONG(now - captain.nextDash) < 0 || !captain.ship.alive())
        return false;
    game::Vector at = captain.ship.position();
    game::Vector up = game::unit(at), line = at - threat;
    game::Vector away = game::unit(line - up * game::dot(line, up));
    game::Vector side = game::cross(up, away) * std::array{1.0f, -1.0f}[size_t(rand() & 1)];
    game::Vector heading = game::unit(side * settings.dashSide + away * settings.dashAway);
    constexpr float seconds = float(settings.dashMilliseconds) / 1000.0f;
    game::Vector landing =
        game::unit(at + heading * (settings.dashSpeed * float(settings.dashMilliseconds))) * game::length(at);
    game::Vector chaser = threat + game::unit(line) * std::min(incoming * seconds, game::length(line));
    float guess = 1.0f + settings.misjudgement * (float(rand() % 201) / 100.0f - 1.0f);
    if (incoming > 0.0f && game::distance(landing, chaser) <= settings.reacquireRange * guess)
        return false;
    Barrier* barrier = barrierOf(captain.ship.fighter().raw());
    float cost = barrier ? barrier->strength * settings.dashCost : 0.0f;
    if (barrier && (danger <= cost || barrier->left <= cost))
        return false;
    captain.nextDash = now + settings.dashCooldownMilliseconds;
    captain.dashVelocity = heading * settings.dashSpeed;
    captain.landing = landing;
    captain.dashUntil = now + settings.dashMilliseconds;
    captain.dashing = true;
    for (uint32_t effect : {settings.jumpOut, settings.jumpTrail})
        flash(captain.ship, captain.ship.body().raw(), effect, 1.0f, settings.jumpBurstMilliseconds);
    game::playAudio(settings.jumpSound);
    if (!barrier)
        return true;
    barrier->left -= cost;
    settle(*barrier, true);
    return true;
}

void Shield::reacquire(const Captain& captain) {
    for (int i = strayCount - 1; i >= 0; --i) {
        Stray stray = strays[i];
        if (stray.ship != captain.ship)
            continue;
        game::Tool tool = stray.projectile.tool();
        game::Vector at = captain.ship.position();
        bool caught = stray.projectile.alive() && tool &&
                      game::distance(stray.projectile.position(), at) <= settings.reacquireRange;
        if (caught) {
            void*& source = tool.source();
            void* kept = source;
            source = nullptr;
            map::space::homeProjectile(stray.projectile.raw(), tool.projectileSpeed(), captain.ship.body().raw(), &at);
            source = kept;
        }
        stray.projectile.release();
        stray.ship.release();
        strays[i] = strays[--strayCount];
        strays[strayCount] = Stray{};
    }
}

void Shield::patrol(game::Ship ship, DWORD now) {
    if (LONG(now - nextPatrolJump) < 0 || hopCount >= shield::maximumHops ||
        rand() % 1000 >= settings.patrolJumpChance ||
        std::ranges::find(used(hops, hopCount), ship, &Hop::ship) != used(hops, hopCount).end())
        return;
    nextPatrolJump = now + settings.patrolGapMilliseconds;
    game::Vector at = ship.position();
    game::Vector up = game::unit(at);
    game::Vector aside = game::unit(game::cross(up, game::Vector{up.y, up.z, up.x}));
    float lean = settings.patrolScatter * (float(rand() % 201) / 100.0f - 1.0f);
    game::Vector landing = game::unit(aside * lean - up) * game::length(at);
    ship.addReference();
    hops[hopCount++] = Hop{.ship = ship, .landing = landing, .until = now + settings.dashMilliseconds};
    flash(ship, ship.body().raw(), settings.jumpOut, 1.0f, settings.jumpBurstMilliseconds);
    game::Vector listener = game::shipPosition();
    if (std::min(game::distance(listener, at), game::distance(listener, landing)) <= settings.hearRange)
        game::playAudio(settings.jumpSound);
}

void Shield::weave(DWORD now) {
    for (int i = hopCount - 1; i >= 0; --i) {
        Hop hop = hops[i];
        if (hop.ship.alive() && LONG(now - hop.until) < 0)
            continue;
        if (hop.ship.alive()) {
            map::space::shipSnapTo(hop.ship.raw(), &hop.landing);
            flash(hop.ship, hop.ship.body().raw(), settings.jumpIn, 1.0f, settings.jumpBurstMilliseconds);
        }
        hop.ship.release();
        hops[i] = hops[--hopCount];
        hops[hopCount] = Hop{};
    }
    for (int i = strayCount - 1; i >= 0; --i) {
        Stray stray = strays[i];
        if (stray.projectile.alive() && stray.ship.alive())
            continue;
        stray.projectile.release();
        stray.ship.release();
        strays[i] = strays[--strayCount];
        strays[strayCount] = Stray{};
    }
    for (Captain& captain : used(captains, captainCount)) {
        if (!captain.dashing || !captain.ship.alive())
            continue;
        if (LONG(now - captain.dashUntil) >= 0) {
            captain.dashing = false;
            map::space::shipSnapTo(captain.ship.raw(), &captain.landing);
            flash(captain.ship, captain.ship.body().raw(), settings.jumpIn, 1.0f, settings.jumpBurstMilliseconds);
            reacquire(captain);
            continue;
        }
        game::Vector axis = game::unit(captain.dashVelocity);
        game::Vector& velocity = captain.ship.velocity();
        velocity = velocity - axis * game::dot(velocity, axis) + captain.dashVelocity;
    }
}

bool Shield::onProjectile(void* fired, void* launched, const game::Vector* origin) {
    bool domed = std::ranges::any_of(barriers(), [](const Barrier& barrier) { return bool(barrier.city); });
    if (domed && launched && origin && shotCount < shield::maximumShots) {
        Shot shot{.projectile = game::Projectile(launched), .last = *origin, .shooter = shooterOf(game::Tool(fired))};
        shot.projectile.addReference();
        shots[shotCount++] = shot;
    }
    game::Tool tool(fired);
    void* aimed = tool ? tool.target() : nullptr;
    if (!aimed || std::ranges::find(settings.lockingTools, tool.identifier()) == settings.lockingTools.end())
        return false;
    auto locked = [aimed](const Captain& captain) { return captain.ship.body().raw() == aimed; };
    auto found = std::ranges::find_if(used(captains, captainCount), locked);
    if (found == used(captains, captainCount).end() || rand() % 100 >= settings.decoyChance)
        return false;
    if (!origin || !jump(*found, *origin, tool.maximumDamage(), tool.projectileSpeed()))
        return false;
    flash(found->ship, found->ship.body().raw(), settings.decoy, 1.0f, settings.decoyBurstMilliseconds);
    play(decoyClip, settings.decoySound, settings.decoySoundMilliseconds);
    if (!launched || strayCount >= shield::maximumStrays)
        return true;
    Stray stray{.projectile = game::Projectile(launched), .ship = found->ship};
    stray.projectile.addReference();
    stray.ship.addReference();
    strays[strayCount++] = stray;
    return true;
}

void Shield::land(Flight& flight, DWORD now) {
    flight.missile.release();
    flight.missile = game::Missile();
    flight.target.release();
    flight.target = game::GameData();
    flight.fadeUntil = now + settings.trailMilliseconds;
    if (flight.trail)
        flight.trail.stop();
}

void Shield::guide(DWORD now) {
    for (int i = flightCount - 1; i >= 0; --i) {
        Flight& flight = flights[i];
        if (flight.missile && !flight.missile.alive())
            land(flight, now);
        if (!flight.missile) {
            if (LONG(now - flight.fadeUntil) < 0)
                continue;
            if (flight.trail)
                flight.trail.release();
            flights[i] = flights[--flightCount];
            flights[flightCount] = Flight{};
            continue;
        }
        if (flight.trail)
            game::placeEffect(flight.trail, flight.missile.body().raw(), 1.0f);
        game::Guidance guidance = flight.missile.guidance();
        if (!guidance)
            continue;
        game::Vector at = flight.target.position();
        DWORD passed = now - flight.sampled;
        if (passed >= settings.paceMilliseconds) {
            flight.pace =
                std::max(game::distance(at, flight.seen) * 1000.0f / float(passed), flight.pace * settings.paceKeep);
            flight.seen = at;
            flight.sampled = now;
        }
        float speed = std::clamp(flight.pace * settings.missileChase, settings.missileSpeed, settings.missileTopSpeed);
        bool striking = game::distance(flight.missile.position(), at) < settings.missileStrikeRange;
        guidance.speed() = speed;
        guidance.turnRate() =
            speed / std::array{settings.missileTurnRadius, settings.missileStrikeRadius}[size_t(striking)];
    }
}

void Shield::command(DWORD now) {
    uint32_t owner = game::planetOwnerPoliticalId();
    for (int i = captainCount - 1; i >= 0; --i) {
        game::Ship ship = captains[i].ship;
        if (ship.alive() && ship.fighter().health() > 0.0f)
            continue;
        ship.release();
        captains[i] = captains[--captainCount];
    }
    for (game::Ship ship : game::each<game::Ship>(map::space::ships)) {
        if (!ship || !ship.alive() || ship.politicalId() != owner)
            continue;
        game::Combatant fighter = ship.fighter();
        void* target = fighter.target();
        if (fighter.health() > 0.0f && !target)
            patrol(ship, now);
        if (!(fighter.health() > 0.0f) || !target || !game::playerSide(target))
            continue;
        Captain* captain = captainOf(ship);
        if (!captain)
            continue;
        if (LONG(now - captain->nextMissile) >= 0)
            fireMissile(*captain, game::Combatant(target).owner(), now);
        bool hurt = fighter.health() < fighter.maximumHealth() * settings.raiseBelow;
        if (hurt && !barrierOf(fighter.raw()) && LONG(now - captain->shieldReady) >= 0) {
            captain->shieldReady = now + settings.shipCooldownMilliseconds;
            raiseShip(ship, now);
        }
    }
}

game::City Shield::cityOf(game::GameData part) const {
    uint32_t owner = part.politicalId();
    for (game::City city : game::each<game::City>(map::civilization::cities)) {
        if (!city || !city.alive() || city.politicalId() != owner)
            continue;
        if (city.hall() == part.raw() ||
            std::ranges::any_of(city.buildings(), [&](game::GameData one) { return one.raw() == part.raw(); }) ||
            std::ranges::any_of(city.turrets(), [&](game::Turret one) { return one.raw() == part.raw(); }))
            return city;
    }
    return game::City();
}

void Shield::guardCity(void* combatant, uint32_t politicalId) {
    if (politicalId != game::playerPoliticalId() || !elitePlanet())
        return;
    game::GameData part = game::Combatant(combatant).owner();
    if (!part || part.politicalId() != game::planetOwnerPoliticalId())
        return;
    game::City city = cityOf(part);
    if (!city)
        return;
    DWORD now = GetTickCount();
    auto found = std::ranges::find(used(wards, wardCount), city, &Ward::city);
    if (found == used(wards, wardCount).end()) {
        if (wardCount >= shield::maximumWards)
            return;
        city.addReference();
        wards[wardCount] = Ward{.city = city, .ready = now};
        found = used(wards, ++wardCount).end() - 1;
    }
    if (LONG(now - found->ready) < 0)
        return;
    if (raiseCity(city, now))
        found->ready = now + settings.cityMilliseconds + settings.cityCooldownMilliseconds;
}

bool Shield::absorb(void* combatant, int type, float damage, void* attacker) {
    if (!combatant || type == int(map::identifiers::Damage::Heal) || !(damage > 0.0f) || !game::onPlanet())
        return false;
    Barrier* barrier = barrierOf(combatant);
    if (!barrier)
        return false;
    alert(*barrier, attacker);
    strike(*barrier, damage);
    return true;
}

void Shield::alert(const Barrier& barrier, void* attacker) {
    if (!attacker || !barrier.city)
        return;
    for (game::Turret turret : barrier.city.turrets())
        if (turret.active() && !turret.fighter().target())
            turret.fighter().setTarget(attacker);
}

void Shield::strike(Barrier& barrier, float damage) {
    barrier.left -= damage;
    if (barrier.left <= 0.0f) {
        lower(int(&barrier - barrierList.data()), true);
        return;
    }
    int stage = barrier.stage;
    settle(barrier, true);
    DWORD now = GetTickCount();
    if (barrier.stage != stage || LONG(now - barrier.nextHit) < 0)
        return;
    barrier.nextHit = now + settings.hitMilliseconds;
    burst(barrier, lookOf(barrier).hit, settings.hitBurstMilliseconds);
    game::playAudio(settings.hitSound);
}

Shield::Shooter Shield::shooterOf(game::Tool tool) {
    void* source = tool ? tool.source() : nullptr;
    void* owner = source ? game::callVirtual<void*>(source, 0xb8, uint32_t(Type::GameData)) : nullptr;
    if (!owner)
        return Shooter{};
    return Shooter{.politicalId = game::GameData(owner).askPoliticalId(),
                   .fighter = game::callVirtual<void*>(source, 0xb8, uint32_t(Type::Combatant))};
}

Shield::Barrier* Shield::domeAhead(const game::Vector& from, const game::Vector& to, uint32_t shooter) {
    float step = game::distance(from, to);
    for (Barrier& barrier : barriers()) {
        const game::Vector* middle =
            barrier.city && barrier.city.politicalId() != shooter ? barrier.city.place().position() : nullptr;
        if (!middle)
            continue;
        float radius = cityRadiusOf(barrier.city), was = game::distance(from, *middle),
              left = game::distance(to, *middle);
        bool above = game::dot(to - *middle, game::unit(*middle)) >= 0.0f;
        if (was > radius && left < was && left - radius <= step * settings.shotLookAhead && above)
            return &barrier;
    }
    return nullptr;
}

void Shield::intercept() {
    for (int i = shotCount - 1; i >= 0; --i) {
        Shot& shot = shots[i];
        bool flying = shot.projectile.alive() && shot.projectile.stage() == game::Projectile::flying;
        game::Vector at = flying ? shot.projectile.position() : shot.last;
        Barrier* struck = flying ? domeAhead(shot.last, at, shot.shooter.politicalId) : nullptr;
        shot.last = at;
        if (flying && !struck)
            continue;
        if (struck && shot.projectile.highestDamage() > 0) {
            int low = shot.projectile.lowestDamage(), high = shot.projectile.highestDamage();
            shot.projectile.lowestDamage() = shot.projectile.highestDamage() = 0;
            shot.projectile.flownMilliseconds() += settings.fuseMilliseconds;
            Shooter shooter = shooterOf(shot.projectile.tool());
            game::Vector toward = game::unit(at - shot.last);
            void* victim = struck->parts[0].cast(Type::Combatant);
            float damage = float(low + rand() % (std::max(high - low, 0) + 1));
            alert(*struck, shooter.fighter);
            if (victim && shooter.fighter)
                game::callVirtual<int>(victim, settings.takeDamageSlot, 0.0f, shooter.politicalId,
                                       shot.projectile.damageKind(), &toward, shooter.fighter);
            if (Barrier* standing = barrierOf(victim))
                strike(*standing, damage);
        }
        shot.projectile.release();
        shots[i] = shots[--shotCount];
        shots[shotCount] = Shot{};
    }
}

Shield::Cut Shield::cutBeam(void* view) {
    Cut cut;
    game::Beam beam = game::BeamView(static_cast<char*>(view)).beam();
    game::Vector* const first = beam ? beam.firstPoint() : nullptr;
    int count = first ? int(beam.pastLastPoint() - first) : 0;
    if (count < 2 || count > shield::maximumBeamPoints)
        return cut;
    uint32_t shooter = shooterOf(beam.tool()).politicalId;
    for (const Barrier& barrier : barriers()) {
        if (!barrier.city || barrier.city.politicalId() == shooter || !outside(*first, barrier.city))
            continue;
        float passed = 0.0f;
        for (int index = 0; index + 1 < count; ++index) {
            float length = game::distance(first[index], first[index + 1]);
            std::optional<float> entry =
                length > 0.0f
                    ? domeEntry(first[index], (first[index + 1] - first[index]) * (1.0f / length), barrier.city)
                    : std::nullopt;
            if (!entry || *entry > length) {
                passed += length;
                continue;
            }
            cut.first = first;
            cut.count = count;
            std::copy_n(first, count, cut.kept.begin());
            float whole = passed + *entry;
            int piece = 0;
            float before = 0.0f;
            for (int point = 0; point < count; ++point) {
                float wanted = whole * float(point) / float(count - 1);
                float along = game::distance(cut.kept[size_t(piece)], cut.kept[size_t(piece) + 1]);
                while (piece < index && before + along < wanted) {
                    before += along;
                    ++piece;
                    along = game::distance(cut.kept[size_t(piece)], cut.kept[size_t(piece) + 1]);
                }
                game::Vector from = cut.kept[size_t(piece)], to = cut.kept[size_t(piece) + 1];
                first[point] = from + (to - from) * (along > 0.0f ? (wanted - before) / along : 0.0f);
            }
            return cut;
        }
    }
    return cut;
}

void Shield::mendBeam(const Cut& cut) {
    if (cut.first)
        std::copy_n(cut.kept.begin(), cut.count, cut.first);
}

const Shield::Barrier* Shield::pointedDome() {
    game::Projection projection(game::activeViewer());
    HWND window = GetForegroundWindow();
    DWORD process = 0;
    POINT pointer{};
    RECT client{};
    if (!projection.usable() || !window || !GetWindowThreadProcessId(window, &process) ||
        process != GetCurrentProcessId() || !GetCursorPos(&pointer) || !ScreenToClient(window, &pointer) ||
        !GetClientRect(window, &client) || client.right <= 0 || client.bottom <= 0)
        return nullptr;
    game::Vector toward =
        projection.through(float(pointer.x) / float(client.right), float(pointer.y) / float(client.bottom));
    const Barrier* nearest = nullptr;
    float least = 0.0f;
    for (const Barrier& barrier : barriers()) {
        std::optional<float> entry = barrier.city ? domeEntry(projection.eye(), toward, barrier.city) : std::nullopt;
        if (entry && (!nearest || *entry < least)) {
            nearest = &barrier;
            least = *entry;
        }
    }
    return nearest;
}

bool Shield::buildPlate() {
    game::AlliesPanel panel = game::SpaceInterface::get().alliesPanel();
    game::Window root = panel ? panel.findWindow(game::AlliesPanel::containerControl) : game::Window();
    for (int depth = 0; root && root.parent() && depth < settings.rootDepth; ++depth)
        root = root.parent();
    if (!root)
        return false;
    root.addReference();
    plate.root = root;
    std::array<game::Key, 4> pictures{settings.platePictures[0], settings.gaugeIcon, settings.platePictures[1],
                                      settings.platePictures[2]};
    for (size_t index = 0; index < pictures.size(); ++index) {
        plate.images[index] = game::loadPicture(pictures[index]);
        plate.looks[index] = game::newPictureLook(plate.images[index]);
    }
    plate.card = game::newOverlay(settings.plateArea);
    plate.parts = {game::newOverlay(settings.plateIcon), game::newOverlay(settings.plateTrack)};
    plate.fill = game::newOverlay(settings.plateFill);
    std::array<game::Window, 4> windows{plate.card, plate.parts[0], plate.parts[1], plate.fill};
    bool whole = std::ranges::all_of(windows, [](game::Window window) { return bool(window); }) &&
                 std::ranges::all_of(plate.looks, [](game::StandardDrawable look) { return bool(look); });
    if (!whole) {
        dropPlate();
        return false;
    }
    for (size_t index = 0; index < windows.size(); ++index)
        windows[index].setDrawable(plate.looks[index]);
    plate.parts[1].add(plate.fill);
    for (game::Window part : plate.parts)
        plate.card.add(part);
    plate.card.setFlag(game::Window::visibleFlag, false);
    root.add(plate.card);
    plate.placed = true;
    return true;
}

void Shield::dropPlate() {
    if (plate.placed)
        plate.root.dispose(plate.card);
    for (game::Window window : {plate.card, plate.parts[0], plate.parts[1], plate.fill, plate.root})
        if (window)
            window.release();
    for (game::StandardDrawable look : plate.looks)
        if (look)
            look.release();
    for (void* image : plate.images)
        game::releasePicture(image);
    plate = Plate{};
}

void Shield::showPlate() {
    const Barrier* barrier = pointedDome();
    const game::Vector* middle = barrier ? barrier->city.place().position() : nullptr;
    game::Projection projection(game::activeViewer());
    std::optional<game::Projection::ScreenPoint> point =
        middle ? projection.project(*middle + game::unit(*middle) * (cityRadiusOf(barrier->city) * settings.plateLift))
               : std::nullopt;
    bool shown = point && (plate.card || buildPlate());
    const game::Rectangle* screen = shown ? plate.root.area() : nullptr;
    if (plate.card)
        plate.card.setFlag(game::Window::visibleFlag, screen != nullptr);
    if (!screen)
        return;
    float width = settings.plateArea.right, height = settings.plateArea.bottom;
    float screenWidth = screen->right - screen->left, screenHeight = screen->bottom - screen->top;
    float left = std::clamp(point->x * screenWidth / projection.viewportWidth() - width / 2.0f, settings.plateMargin,
                            std::max(settings.plateMargin, screenWidth - width)),
          top = std::clamp(point->y * screenHeight / projection.viewportHeight() - settings.plateGap - height,
                           settings.plateMargin, std::max(settings.plateMargin, screenHeight - height));
    plate.card.setArea({left, top, left + width, top + height});
    const game::Rectangle& fill = settings.plateFill;
    float share = std::clamp(barrier->left / barrier->strength, 0.0f, 1.0f);
    plate.fill.setArea({fill.left, fill.top, fill.left + (fill.right - fill.left) * share, fill.bottom});
}

bool Shield::onDamage(void* combatant, uint32_t politicalId, int type, float damage, void* attacker) {
    if (absorb(combatant, type, damage, attacker))
        return true;
    guardCity(combatant, politicalId);
    return false;
}

bool Shield::onShipDamage(void* combatant, int type, float damage) {
    return absorb(combatant, type, damage, nullptr);
}

const Shield::Barrier* Shield::barrierFor(game::GameData hovered) {
    for (const Barrier& barrier : barriers()) {
        if (barrier.ship ? barrier.ship.raw() == hovered.raw() : barrier.city.raw() == hovered.raw())
            return &barrier;
        for (game::GameData part : used(barrier.parts, barrier.partCount))
            if (part.raw() == hovered.raw())
                return &barrier;
    }
    return nullptr;
}

void Shield::dropGauge() {
    for (game::Window shown : {gauge.frame, gauge.icon})
        if (shown) {
            gauge.host.dispose(shown);
            shown.release();
        }
    if (gauge.fill)
        gauge.fill.release();
    if (gauge.look)
        gauge.look.release();
    game::releasePicture(gauge.image);
    for (const Moved& moved : std::span(gauge.pushed).first(size_t(gauge.pushedCount))) {
        moved.window.setArea(moved.area);
        moved.window.release();
    }

    if (gauge.plate.window) {
        gauge.plate.window.setArea(gauge.plate.area);
        gauge.plate.window.release();
    }
    for (game::Window held : {gauge.host, gauge.bar})
        if (held)
            held.release();
    gauge = Gauge{};
}

void Shield::showGauge() {
    game::GameData hovered = game::hoveredObject();
    const Barrier* barrier = hovered ? barrierFor(hovered) : nullptr;
    char* rollover = barrier && barrier->ship ? rolloverOf(hovered) : nullptr;
    game::Window bar = rollover ? game::Window(game::field<char*>(rollover, 0x70)) : game::Window();
    if (bar != gauge.bar)
        dropGauge();
    game::Window plate = bar ? game::Window(game::field<char*>(rollover, 0x58)) : game::Window();
    game::Window blue = bar ? game::Window(game::field<char*>(rollover, 0x68)) : game::Window();
    const game::Rectangle* area = bar ? bar.area() : nullptr;
    if (!area || !plate || !plate.area() || !blue)
        return;
    game::Rectangle healthBar = *area;
    float width = healthBar.right - healthBar.left, height = healthBar.bottom - healthBar.top;
    if (!gauge.frame) {
        game::Window host = bar.parent();
        if (!host || plate.parent() != host)
            return;
        game::Rectangle plateArea = *plate.area();
        float row = height + settings.gaugeRowExtra;
        game::Window frame =
            game::newOverlay({healthBar.left, healthBar.top - row, healthBar.right, healthBar.bottom - row});
        game::Window fill = frame ? game::newOverlay({0.0f, 0.0f, 0.0f, height}) : game::Window();
        if (!fill) {
            if (frame)
                frame.release();
            return;
        }
        frame.setDrawable(bar.drawable());
        fill.setDrawable(blue.drawable());
        frame.add(fill);
        host.add(frame);
        host.addReference();
        bar.addReference();
        plate.addReference();
        gauge = Gauge{.host = host, .bar = bar, .frame = frame, .fill = fill, .plate = {plate, plateArea}};
        float threshold = healthBar.top - plateArea.top - settings.gaugeRowExtra;
        plate.forEachChild([&](game::Window child) {
            const game::Rectangle* place = child.area();
            if (!place || place->top < threshold || gauge.pushedCount >= int(gauge.pushed.size()))
                return;
            child.addReference();
            gauge.pushed[size_t(gauge.pushedCount++)] = Moved{child, *place};
            child.setArea({place->left, place->top + row, place->right, place->bottom + row});
        });
        plate.setArea({plateArea.left, plateArea.top - row, plateArea.right, plateArea.bottom});
        void* image = game::loadPicture(settings.gaugeIcon);
        game::StandardDrawable look = game::newPictureLook(image);
        float right = healthBar.left - settings.gaugeIconGap;
        game::Rectangle place{right - settings.gaugeIconSize, healthBar.top - row - settings.gaugeIconGap + 2.0f, right,
                              healthBar.top - row - settings.gaugeIconGap + 2.0f + settings.gaugeIconSize};
        if (gauge.pushedCount) {
            const game::Rectangle& plus = gauge.pushed[0].area;
            place = {plateArea.left + plus.left, plateArea.top + plus.top - row, plateArea.left + plus.right,
                     plateArea.top + plus.bottom - row};
        }
        game::Window icon = look ? game::newOverlay(place) : game::Window();
        if (icon) {
            icon.setDrawable(look);
            host.add(icon);
        }
        gauge.icon = icon;
        gauge.look = look;
        gauge.image = image;
    }
    float share = std::clamp(barrier->left / barrier->strength, 0.0f, 1.0f);
    gauge.fill.setArea({0.0f, 0.0f, width * share, height});
}

void Shield::releaseAll() {
    dropGauge();
    dropPlate();
    for (Shot& shot : used(shots, shotCount)) {
        shot.projectile.release();
        shot = Shot{};
    }
    shotCount = 0;
    while (barrierCount)
        lower(barrierCount - 1, false);
    clearEnded(GetTickCount() + settings.endMilliseconds + 1);
    moveBursts(GetTickCount() + settings.burstMilliseconds + 1);
    for (Captain& captain : used(captains, captainCount))
        captain.ship.release();
    captainCount = 0;
    for (Stray& stray : used(strays, strayCount)) {
        stray.projectile.release();
        stray.ship.release();
        stray = Stray{};
    }
    strayCount = 0;
    for (Hop& hop : used(hops, hopCount)) {
        hop.ship.release();
        hop = Hop{};
    }
    hopCount = 0;
    for (Flight& flight : used(flights, flightCount)) {
        if (flight.missile)
            land(flight, 0);
        if (flight.trail) {
            flight.trail.halt();
            flight.trail.release();
        }
        flight = Flight{};
    }
    flightCount = 0;
    for (Ward& ward : used(wards, wardCount))
        ward.city.release();
    wardCount = 0;
    for (Clip* clip : {&decoyClip, &launchClip})
        quiet(*clip);
}

void Shield::onLeavePlanet(bool) {
    releaseAll();
}

void Shield::holdDeadlines(DWORD step) {
    for (Barrier& barrier : barriers()) {
        barrier.until += step;
        barrier.nextHit += step;
    }
    for (Captain& captain : used(captains, captainCount)) {
        captain.nextMissile += step;
        captain.shieldReady += step;
        captain.nextDash += step;
        captain.dashUntil += step;
    }
    for (Ward& ward : used(wards, wardCount))
        ward.ready += step;
    for (Flight& flight : used(flights, flightCount)) {
        flight.sampled += step;
        flight.fadeUntil += step;
    }
    for (Hop& hop : used(hops, hopCount))
        hop.until += step;
    nextPatrolJump += step;
}

void Shield::tick() {
    if (!game::onPlanet()) {
        if (barrierCount || burstCount || captainCount || wardCount || flightCount || strayCount || hopCount ||
            shotCount || endedCount || plate.root)
            releaseAll();
        lastTick = 0;
        return;
    }
    DWORD now = GetTickCount();
    DWORD step = lastTick ? now - lastTick : 0;
    lastTick = now;
    if (game::paused())
        holdDeadlines(step);
    for (Clip* clip : {&decoyClip, &launchClip})
        if (clip->heard && LONG(now - clip->until) >= 0)
            quiet(*clip);
    moveBursts(now);
    clearEnded(now);
    guide(now);
    showGauge();
    showPlate();
    DWORD aged = std::array{step, DWORD(0)}[size_t(game::paused())];
    for (int i = barrierCount - 1; i >= 0; --i) {
        Barrier& barrier = barrierList[i];
        game::GameData holder = barrier.ship ? game::GameData(barrier.ship) : game::GameData(barrier.city);
        barrier.left -= barrier.strength * float(aged) /
                        float(barrier.ship ? settings.shipMilliseconds : settings.cityMilliseconds);
        if (!holder.alive() || !(barrier.left > 0.0f) || LONG(now - barrier.until) >= 0) {
            lower(i, false);
            continue;
        }
        settle(barrier, false);
        if (!barrier.ship)
            continue;
        if (barrier.dome)
            game::placeEffect(barrier.dome, barrier.ship.body().raw(), 1.0f);
        if (barrier.cracks)
            game::placeEffect(barrier.cracks, barrier.ship.body().raw(), settings.shipScale);
    }
    if (game::paused())
        return;
    intercept();
    weave(now);
    if (LONG(now - nextThink) < 0)
        return;
    nextThink = now + settings.thinkMilliseconds;
    if (elitePlanet())
        command(now);
}
}

