#pragma once

class Layer {
public:
    virtual ~Layer() = default;

    virtual void onUpdate() {}
    virtual void onRender() {}
};