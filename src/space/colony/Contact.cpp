
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <algorithm>
#include <cstdint>

module beatuy.feature.colony;

import beatuy.game;

namespace feature {
void Contact::onHeader(void* screen, uint32_t politicalId) {
    if (!game::inSpace())
        return;
    game::Window face = game::CommScreen(screen).layout().findWindow(game::CommScreen::moodControl);
    game::Window header = face ? face.parent() : game::Window();
    if (!header || !borrow())
        return;
    if (badge.parent() != header)
        move(header);
    if (face != mood) {
        mood = face;
        moodArea = *face.area();
    }
    game::Empire empire = game::empireOf(politicalId);
    game::AnimatedIcon icon = game::AnimatedIcon::of(badge);
    int frame = empire ? std::min(icon.frames() - 1, game::empireLevel(empire) + 1) : -1;
    badge.setFlag(game::Window::visibleFlag, frame >= 0);
    icon.show(frame);
    makeRoom(frame >= 0);
    align();
}

void Contact::onUnload() {
    if (mood)
        makeRoom(false);
    mood = game::Window();
    if (badge && badge.parent() != home)
        move(home);
}

void Contact::tick() const {
    if (!badge || !mood || badge.parent() == home)
        return;
    if (game::conversing()) {
        align();
        return;
    }
    if (badge.flags() & game::Window::visibleFlag) {
        badge.setFlag(game::Window::visibleFlag, false);
        makeRoom(false);
    }
}

bool Contact::borrow() {
    if (badge || tried)
        return bool(badge);
    tried = true;
    tooltip = game::newLayout("UI/Contact Layout");
    if (!tooltip.load(settings.tooltipLayout, settings.layoutGroup, false, settings.layoutLayer))
        return false;
    game::Window found = tooltip.findWindow(settings.badgeControl);
    if (!found || !found.parent() || !game::AnimatedIcon::of(found))
        return false;
    badge = found;
    home = found.parent();
    return true;
}

void Contact::move(game::Window into) const {
    badge.addReference();
    if (game::Window holder = badge.parent())
        holder.remove(badge);
    into.add(badge);
    into.toFront(badge);
    badge.release();
}

void Contact::makeRoom(bool aside) const {
    float shift = aside ? settings.moodShift : 0.0f;
    mood.setArea({moodArea.left - shift, moodArea.top, moodArea.right - shift, moodArea.bottom});
}

void Contact::align() const {
    const game::Rectangle* face = mood.realArea();
    const game::Rectangle* own = badge.area();
    if (!face || !own)
        return;
    float width = own->right - own->left, height = own->bottom - own->top;
    float left = face->right + settings.badgeSpacing;
    float top = (face->top + face->bottom - height) / 2.0f;
    game::Rectangle place{left, top, left + width, top + height};
    if (place != *own)
        badge.setArea(place);
}
}

