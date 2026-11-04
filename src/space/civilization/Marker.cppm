
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <Windows.h>
#include <array>

export module beatuy.feature.civilization:marker;

import :render;
import :settings;
import beatuy.game;

namespace civilization {
class Marker {
public:
    void begin();
    void add(game::GameData unit);
    void end();

private:
    struct Ring {
        std::array<game::Vector, settings.rings.points + 1> points;
    };

    struct Vertex {
        float x, y, z, reciprocalHomogeneousW;
        DWORD color;
    };

    class Layer {
    public:
        explicit Layer(Marker& marker) : owner(marker) {}

        virtual int addReference() {
            return 2;
        }

        virtual int release() {
            return 1;
        }

        virtual ~Layer() = default;

        virtual void drawLayer(int, int, void**, void*);

    private:
        Marker& owner;
    };

    game::Viewer findViewer();
    void push(const Vertex& vertex);
    void segment(float fromX, float fromY, float toX, float toY, float halfWidth, DWORD core, DWORD edge);
    void drawRing(const game::Projection& projection, const Ring& ring, float pulse);
    void draw();
    void ensure();

    static constexpr unsigned maximumVertices = unsigned(settings.limits.maximumRings * settings.rings.points * 18);

    std::array<Ring, settings.limits.maximumRings> rings;
    int ringCount = 0;
    DWORD updated = 0;
    game::Vector eye{};
    bool eyeChecked = false, eyeKnown = false;
    std::array<Vertex, maximumVertices> vertices;
    unsigned vertexCount = 0;
    int layerIndex = -1;
    Layer layer{*this};
};
}

