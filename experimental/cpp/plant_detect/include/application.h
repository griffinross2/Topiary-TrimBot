#pragma once

#include <layer.h>

#include <memory>
#include <type_traits>
#include <vector>

#include <GLFW/glfw3.h>
#ifdef __linux__
#include "libcamera/libcamera.h"
#endif

class Application {
public:
    int init();
    int run();
    void shutdown();

    template <typename T>
        requires std::is_base_of_v<Layer, T>
    void pushLayer() {
        m_layerStack.push_back(std::make_unique<T>());
    }
#ifdef __linux__
    std::shared_ptr<libcamera::CameraManager>& getCameraManager();
#endif

private:
    std::vector<std::unique_ptr<Layer>> m_layerStack;
    GLFWwindow* m_window;
#ifdef __linux__
    std::shared_ptr<libcamera::CameraManager> m_cm;
#endif
};
