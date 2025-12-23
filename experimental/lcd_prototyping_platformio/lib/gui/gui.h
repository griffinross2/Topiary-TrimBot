#ifndef GUI_H
#define GUI_H

#include "font.h"

#include <vector>
#include <string>
#include <memory>

#include "fonts/arial.h"

class Scene;

class SceneObject {
public:
    SceneObject(Scene* parent);

    void trigger_redraw();
    void set_visible(bool visible);
    virtual void redraw();

protected:
    bool m_visible = true;
    Scene* m_parent = nullptr;
};

class Scene {
public:
    Scene();

    void add_object(std::shared_ptr<SceneObject> obj);
    void redraw();

private:
    std::vector<std::shared_ptr<SceneObject>> m_objects;
};

class Label : public SceneObject {
public:
    Label(Scene* parent, int x, int y);
    Label(Scene* parent, int x, int y, std::string text);
    Label(Scene* parent, int x, int y, std::string text, int size);
    Label(Scene* parent, int x, int y, std::string text, int size,
          uint32_t color);
    Label(Scene* parent, int x, int y, std::string text, int size,
          uint32_t color, const Font* font);

    void set_position(int x, int);
    void set_text(std::string text);
    void set_color(uint32_t color);
    void set_font(const Font* font);

    void redraw() override;

private:
    int m_x = 0;
    int m_y = 0;
    std::string m_text = "";
    int m_size = 24;
    uint32_t m_color = 0xFF000000;
    const Font* m_font = &ARIAL;
};

class Button : public SceneObject {
public:
    Button(Scene* parent, int x, int y, int w, int h);

    void redraw() override;

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
};

#endif  // GUI_H