
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

export module beatuy.core.feature;

export {
template <class Type>
concept Feature = requires(Type& feature) { feature.tick(); };

template <class Type>
concept LeavesPlanet = Feature<Type> && requires(Type& feature, bool destroyed) { feature.onLeavePlanet(destroyed); };

template <class Type>
concept TakesClicks = Feature<Type> && requires(Type& feature, int button) { feature.onMouseDown(button); };
}

