#include "application.h"

#include "block.h"

#include <memory>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static int imgui_init(GLFWwindow*& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    io.Fonts->AddFontFromFileTTF("fonts/Roboto/static/Roboto-Regular.ttf");

    return 0;
}

int Application::init() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    if (!glfwInit())
        return -1;

    m_window =
        glfwCreateWindow(1920, 1080, "Plant Detection Test", nullptr, nullptr);

    if (!m_window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(m_window);

    imgui_init(m_window);

    glfwSwapInterval(1);  // vsync

    return 0;
}

int Application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        // App update
        for (auto& layer : m_layerStack) {
            layer->onUpdate();
        }

        updateBlocks();

        ImGui::NewFrame();

        // App render
        for (auto& layer : m_layerStack) {
            layer->onRender();
        }

        renderBlocks();

        renderBlockConnections();

        ImGui::Render();

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    return 0;
}

void Application::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}