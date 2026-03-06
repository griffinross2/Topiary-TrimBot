#ifndef GUI_H
#define GUI_H

#include "font.h"

#include "fonts/arial.h"
#include "lcd.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>

typedef enum {
    LABEL_ALIGN_LEFT,
    LABEL_ALIGN_CENTER,
    LABEL_ALIGN_RIGHT,
} LabelAlignment;

class Scene;

class SceneObject {
public:
    SceneObject(Scene* parent, bool clickable = false);

    void set_visible(bool visible);
    bool is_clickable() { return m_clickable; }
    bool is_visible() { return m_visible; }

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
    void add_dialog_object(std::shared_ptr<SceneObject> obj);
    void redraw();
    std::vector<std::shared_ptr<SceneObject>>& get_objects() {
        return m_objects;
    }
    std::vector<std::shared_ptr<SceneObject>>& get_dialog_objects() {
        return m_dialog_objects;
    }
    void clear_objects() { m_objects.clear(); }
    void clear_dialog_objects() { m_dialog_objects.clear(); }
    void set_dialog_active(bool active) { m_dialog_active = active; }
    bool is_dialog_active() { return m_dialog_active; }

private:
    ColorRGB565 m_background_color = 0xFFFF;
    std::vector<std::shared_ptr<SceneObject>> m_objects;
    std::vector<std::shared_ptr<SceneObject>> m_dialog_objects;
    bool m_dialog_active = false;
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
    ColorRGB565 m_color = 0;
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
    void set_alignment(LabelAlignment alignment) { m_alignment = alignment; }

    void redraw() override;
    void handle_press(int x, int y) override {}
    void handle_release(int x, int y) override {}

private:
    int m_x = 0;
    int m_y = 0;
    std::string m_text = "";
    int m_size = 24;
    ColorRGB565 m_color = 0;
    const Font* m_font = &ARIAL;
    int m_text_width = 0;
    LabelAlignment m_alignment = LABEL_ALIGN_LEFT;
};

class Button : public SceneObject {
public:
    Button(Scene* parent, int x, int y, int w, int h);

    void redraw() override;
    void handle_press(int x, int y) override;
    void handle_release(int x, int y) override;
    void bg_off() { m_background_on = false; }
    void set_on_click(std::function<void(int, int)> on_click) {
        m_on_click = on_click;
    }

private:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    bool m_pressed = false;
    bool m_background_on = true;
    std::function<void(int, int)> m_on_click;
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
void gui_render(unsigned int target_fps = 60);

#endif  // GUI_H