
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

export module beatuy.core.mod;

export import beatuy.feature.civilization;
export import beatuy.feature.colony;
export import beatuy.feature.escort;
export import beatuy.feature.exit;
export import beatuy.feature.generator;
export import beatuy.feature.glow;
export import beatuy.feature.orders;
export import beatuy.feature.radio;
export import beatuy.feature.travel;
export import beatuy.feature.tribe;

export class Mod {
public:
    static Mod& get();

    feature::Picker picker;
    feature::Travel travel;
    feature::QuickExit quickExit;
    feature::Civilization civilization;
    feature::Colony colony;
    feature::Generator generator;
    feature::Tribe tribe;
    feature::Escort escort;
    feature::Orders orders;
    feature::Glow glow;
    feature::Radio radio;

    void tick();
    void leavePlanet(bool destroyed);
    void mouseDown(int button);

private:
    Mod() = default;
};

