#ifndef GUI_H
#define GUI_H

#include "font.h"

#include "fonts/arial.h"
#include "lcd.h"

#include <vector>
#include <string>
#include <memory>

class Scene;

class SceneObject {
public:
    SceneObject(Scene* parent, bool clickable = false);

    void set_visible(bool visible);
    bool is_clickable() { return m_clickable; }

    virtual void redraw() {}
    virtual void handle_press(int x, int y) {}
    virtual void handle_release(int x, int y) {}

protected:
    Scene* m_parent = nullptr;
    bool m_clickable = false;
    bool m_visible = true;
};

class Scene {
public:
    Scene();
    Scene(ColorRGB888 background_color);

    void add_object(std::shared_ptr<SceneObject> obj);
    void redraw();
    std::vector<std::shared_ptr<SceneObject>>& get_objects() {
        return m_objects;
    }

private:
    ColorRGB332 m_background_color = 0xFF;
    std::vector<std::shared_ptr<SceneObject>> m_objects;
};

class Rectangle : public SceneObject {
public:
    Rectangle(Scene* parent, int x, int y, int w, int h, ColorRGB888 color);

    void set_position(int x, int y);
    void set_size(int w, int h);
    void set_color(ColorRGB888 color);

    void redraw() override;
    void handle_press(int x, int y) override {}
    void handle_release(int x, int y) override {}

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    ColorRGB332 m_color = 0;
};

class Label : public SceneObject {
public:
    Label(Scene* parent, int x, int y);
    Label(Scene* parent, int x, int y, std::string text);
    Label(Scene* parent, int x, int y, std::string text, int size);
    Label(Scene* parent, int x, int y, std::string text, int size,
          ColorRGB888 color);
    Label(Scene* parent, int x, int y, std::string text, int size,
          ColorRGB888 color, const Font* font);

    void set_position(int x, int);
    void set_text(std::string text);
    void set_color(ColorRGB888 color);
    void set_font(const Font* font);

    void redraw() override;
    void handle_press(int x, int y) override {}
    void handle_release(int x, int y) override {}

private:
    int m_x = 0;
    int m_y = 0;
    std::string m_text = "";
    int m_size = 24;
    ColorRGB332 m_color = 0;
    const Font* m_font = &ARIAL;
};

class Button : public SceneObject {
public:
    Button(Scene* parent, int x, int y, int w, int h);

    void redraw() override;
    void handle_press(int x, int y) override;
    void handle_release(int x, int y) override;

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_pressed = false;
};

typedef struct {
    bool pressed;
    int x;
    int y;
} TouchState;

void gui_set_current_scene(Scene* scene);
Scene* gui_get_current_scene();
void gui_touch_update();
void gui_touch_irq();

void gui_update();
void gui_render();

#endif  // GUI_H