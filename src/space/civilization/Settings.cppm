
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <cstdint>

export module beatuy.feature.civilization:settings;

namespace civilization {
struct Settings {
    struct Pace {
        DWORD thinkMilliseconds = 200, assertMilliseconds = 250;
    } pace;

    struct Hunt {
        DWORD moveMilliseconds = 2500;
        float verticalReach = 0.25f, approach = 0.7f, standOff = 0.75f, replan = 40.0f;
        float detect = 250.0f, shipReach = 1.8f;
        float closer = 3.0f;
        DWORD outrunMilliseconds = 6000, repullMilliseconds = 15000;
        int pullPerCity = 6;
        float pullRadius = 450.0f, squadRadius = 100.0f;
        float hallMatch = 30.0f;
    } hunt;

    struct Mood {
        float hitScale = 0.5f;
        DWORD hitPenaltyMilliseconds = 500;
        float thanksScale = 0.5f;
        DWORD thanksMilliseconds = 1000;
        DWORD observeMilliseconds = 60000;
        float hostileStanding = -3.0f, peaceStanding = 0.5f;
        float loiterStepSeconds = 8.0f, loiterScale = 1.0f, warFloor = -6.0f;
        int loiterMinimumSteps = 6, loiterMaximumSteps = 12;
        float breachScale = 2.0f;
    } mood;

    struct Command {
        int rallyPerCity = 3, defenders = 4, wave = 8, waveMinimum = 3;
        float rallyRadius = 350.0f, rallyTurrets = 350.0f, raidRadius = 220.0f, defendRadius = 500.0f;
        DWORD rallyMilliseconds = 30000, threatMilliseconds = 10000, defenseGapMilliseconds = 12000,
              offenseMilliseconds = 80000, offenseJitterMilliseconds = 40000, offenseFirstMilliseconds = 30000,
              orderScanMilliseconds = 2000;
    } command;

    struct Belief {
        DWORD chaseMilliseconds = 4000;
        float contact = 6.0f;
    } belief;

    struct Push {
        DWORD gapMilliseconds = 500;
        float scale = 0.5f;
    } push;

    struct Bribe {
        float aloneRadius = 200.0f;
        DWORD aloneMilliseconds = 2000, offerMilliseconds = 47000, offerGapMilliseconds = 30000;
        float textSeconds = 40.0f, seconds = 45.0f;
        int least = 250, most = 1000, step = 50, chance = 60;
        DWORD beamMilliseconds = 300;
        uint32_t sound = 0x5e316f77, toggleStrategy = 0xa5d0d08f;
    } bribe;

    struct Super {
        DWORD thinkMilliseconds = 3000, civilizationGapMilliseconds = 45000, planetGapMilliseconds = 15000,
              graceMilliseconds = 15000, harmMilliseconds = 15000, blastMilliseconds = 5000;
        float cityExtent = 60.0f, blastMargin = 15.0f;
        float reach = 300.0f, swarm = 200.0f, shipDivisor = 4.0f, staticDrain = 0.3f, speed = 150.0f,
              flightSlack = 1.5f;
        float woundShare = 0.75f;
        DWORD woundReactionMilliseconds = 500;
        float auraRadius = 60.0f, engagedReach = 90.0f, worthIt = 2.5f, leadMiss = 0.5f;
        float velocitySmoothing = 4.0f;
    } super;

    struct Design {
        int poolSize = 3;
        DWORD pickGapMilliseconds = 3000;
    } design;

    struct Limits {
        int maximumCities = 48, maximumCivilizations = 8, maximumPulled = 96, maximumTurrets = 64, maximumHalls = 64,
            maximumEscorts = 8, maximumHits = 16, maximumBeams = 32, maximumSpared = 64, maximumAsked = 128,
            maximumStruck = 128, maximumShots = 16, maximumJobs = 32, maximumPicked = 512, maximumRings = 24;
    } limits;

    struct Rings {
        int points = 64;
        float width = 2.6f, lift = 0.6f, feather = 1.5f, horizon = 1.0f, horizonRing = 5.0f;
        uint32_t color = 0xD860FF7A;
        DWORD staleMilliseconds = 500;
    } rings;
};

inline constexpr Settings settings{};
}

