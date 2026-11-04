
// Beatuy!SpaceStage — © 2026 Sheroll s.r.o® All rights reserved.
// Sheroll Non-Commercial License 1.0: see LICENSE in the project root.

module;
#include <cstdint>
#include <span>

export module beatuy.game.objects:camera;

import :view;
import beatuy.game.math;
import beatuy.game.memory;

export namespace game {
class Camera : public View {
public:
    using View::View;
    float& climb() const {
        return at<float>(0x8c);
    }
    float& zoomTarget() const {
        return at<float>(0xb4);
    }
    bool& discreteZoom() const {
        return at<bool>(0xd7);
    }
};

struct Viewport {
    uint32_t left, top, width, height;
    float nearDepth, farDepth;
};

class RenderCamera : public View {
public:
    using View::View;
    bool parallel() const {
        return at<int>(0x6c) == 2;
    }
    std::span<const float, 2> viewOffset() const {
        return std::span<const float, 2>(&at<float>(0x54), 2);
    }
    std::span<const float, 2> viewWindow() const {
        return std::span<const float, 2>(&at<float>(0x5c), 2);
    }
    float nearPlane() const {
        return at<float>(0x70);
    }
    const Viewport& viewport() const {
        return at<Viewport>(0x78);
    }
};

class Viewer : public View {
public:
    using View::View;
    Vector row(int index) const {
        const float* matrix = &at<float>(uint32_t(index) * 16);
        return {matrix[0], matrix[1], matrix[2]};
    }
    RenderCamera camera() const {
        return reference<RenderCamera>(0x170);
    }
};
}

