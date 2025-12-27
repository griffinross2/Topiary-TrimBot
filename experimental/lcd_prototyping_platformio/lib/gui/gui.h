#ifndef GUI_H
#define GUI_H

#include "font.h"

#include <vector>
#include <string>
#include <memory>

#include "fonts/arial.h"

typedef struct {
    int xl;
    int xr;
    int yb;
    int yt;
} Bounds;

class Scene;

class SceneObject {
public:
    SceneObject(Scene* parent, bool clickable = false);

    void trigger_redraw();
    void set_visible(bool visible);
    bool is_clickable() { return m_clickable; }

    virtual void redraw() {}
    virtual void handle_press(int x, int y) {}
    virtual void handle_release(int x, int y) {}
    virtual Bounds calc_bounds() { return Bounds{0,0,0,0}; }

protected:
    Scene* m_parent = nullptr;
    bool m_clickable = false;
    bool m_visible = true;
};


typedef struct {
    std::vector<std::shared_ptr<SceneObject>> objects;
    Bounds bounds;
} DrawGroup;

class Scene {
public:
    Scene();

    void add_object(std::shared_ptr<SceneObject> obj);
    void redraw(SceneObject* obj);
    std::vector<std::shared_ptr<SceneObject>>& get_objects() { return m_objects; }

private:
    std::vector<DrawGroup> create_draw_groups();

    std::vector<DrawGroup> m_draw_groups;
    std::vector<std::shared_ptr<SceneObject>> m_objects;
};

class Label : public SceneObject {
public:
    Label(Scene* parent, int x, int y);
    Label(Scene* parent, int x, int y, std::string text);
    Label(Scene* parent, int x, int y, std::string text, int size);
    Label(Scene* parent, int x, int y, std::string text, int size,
          uint8_t color);
    Label(Scene* parent, int x, int y, std::string text, int size,
          uint8_t color, const Font* font);

    void set_position(int x, int);
    void set_text(std::string text);
    void set_color(uint8_t color);
    void set_font(const Font* font);

    void redraw() override;
    void handle_press(int x, int y) override {}
    void handle_release(int x, int y) override {}
    Bounds calc_bounds() override;

private:
    int m_x = 0;
    int m_y = 0;
    std::string m_text = "";
    int m_size = 24;
    uint8_t m_color = 0xF0;
    const Font* m_font = &ARIAL;
};

class Button : public SceneObject {
public:
    Button(Scene* parent, int x, int y, int w, int h);

    void redraw() override;
    void handle_press(int x, int y) override;
    void handle_release(int x, int y) override;
    Bounds calc_bounds() override;

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

void gui_update_loop();

#endif  // GUI_H