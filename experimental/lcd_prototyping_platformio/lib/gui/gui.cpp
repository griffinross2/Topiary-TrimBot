#include "gui.h"

#include "lcd.h"

Scene::Scene() {}

void Scene::add_object(std::shared_ptr<SceneObject> obj) {
    m_objects.push_back(std::move(obj));
    redraw();
}

void Scene::redraw() {
    lcd_clear_foreground();
    for (auto obj : m_objects) {
        obj->redraw();
    }
}

SceneObject::SceneObject(Scene* parent) : m_parent(parent) {}

void SceneObject::trigger_redraw() {
    if (m_parent) {
        m_parent->redraw();
    }
}

void SceneObject::set_visible(bool visible) {
    m_visible = visible;
    trigger_redraw();
}

Label::Label(Scene* parent, int x, int y)
    : SceneObject(parent), m_x(x), m_y(y), m_text("") {}

Label::Label(Scene* parent, int x, int y, std::string text)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             uint32_t color)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size),
      m_color(color) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             uint32_t color, const Font* font)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size),
      m_color(color), m_font(font) {}

void Label::set_position(int x, int y) {
    m_x = x;
    m_y = y;
    trigger_redraw();
}

void Label::set_text(std::string text) {
    m_text = text;
    trigger_redraw();
}

void Label::set_color(uint32_t color) {
    m_color = color;
    trigger_redraw();
}

void Label::set_font(const Font* font) {
    m_font = font;
    trigger_redraw();
}

void Label::redraw() {
    if (m_visible) {
        lcd_draw_text(m_font, m_text.c_str(), m_x, m_y, m_size, m_color);
    }
}

Button::Button(Scene* parent, int x, int y, int w, int h)
    : SceneObject(parent), m_x(x), m_y(y), m_width(w), m_height(h) {}

void Button::redraw() {
    if (m_visible) {
        lcd_draw_rectangle(m_x, m_y, m_width, m_height, 0xFFA0A0A0);
    }
}