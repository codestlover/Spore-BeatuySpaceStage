
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>

module beatuy.feature.civilization;

import beatuy.game;

namespace civilization {
void Designs::queue(uint32_t politicalId, game::Civilization nation, int index) {
    uint32_t tag = politicalId * slots + uint32_t(index);
    std::span<const uint32_t> alreadyPicked = std::span(picked).first(size_t(pickedCount));
    if (std::ranges::find(alreadyPicked, tag) != alreadyPicked.end())
        return;
    auto queued = [&](const Job& job) { return job.politicalId == politicalId && job.index == index; };
    if (std::ranges::any_of(std::span(jobs).first(size_t(jobCount)), queued))
        return;
    if (jobCount < settings.limits.maximumJobs)
        jobs[jobCount++] = Job{politicalId, nation, index};
}

std::optional<Designs::Job> Designs::due(DWORD now) {
    if (!jobCount || LONG(now - nextPick) < 0)
        return std::nullopt;
    Job job = jobs[0];
    std::shift_left(jobs.begin(), jobs.begin() + jobCount, 1);
    --jobCount;
    return job;
}

void Designs::pick(const Job& job, DWORD now) {
    Pool& pool = pools[job.index];
    nextPick = now + settings.design.pickGapMilliseconds;
    if (pickedCount < settings.limits.maximumPicked)
        picked[pickedCount++] = job.politicalId * slots + uint32_t(job.index);
    char* culture = job.nation.culture();
    uint32_t slot = map::civilization::vehicleSlots[job.index];
    game::culturePick(culture, slot);
    const game::Key* design = game::findCreation(culture, slot);
    if (!design || !design->instance)
        return;
    auto same = [design](const game::Key& key) {
        return key.instance == design->instance && key.group == design->group;
    };
    bool known = std::ranges::any_of(std::span(pool.keys).first(size_t(pool.count)), same);
    if (!known && pool.count < settings.design.poolSize)
        pool.keys[pool.count++] = *design;
}

void Designs::forget() {
    jobCount = 0;
}

void Designs::choose(game::City city, int purpose, int locomotion, game::Key& key) {
    if (purpose < 0 || purpose > 2 || locomotion < 0 || locomotion > 2)
        return;
    game::Civilization nation = city.civilization();
    if (!nation)
        return;
    int index = purpose * 3 + locomotion;
    uint32_t slot = map::civilization::vehicleSlots[index];
    game::Key* design = game::findCreation(nation.culture(), slot);
    bool colony = !design || !design->instance || (design->instance == key.instance && design->group == key.group);
    if (!colony) {
        key = *design;
        return;
    }
    Pool& pool = pools[index];
    uint32_t politicalId = city.politicalId();
    if (design && pool.count) {
        *design = pool.keys[rand() % pool.count];
        key = *design;
    }
    queue(politicalId, nation, index);
}
}

