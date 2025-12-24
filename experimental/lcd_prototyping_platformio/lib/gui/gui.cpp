#include "gui.h"

#include "lcd.h"
#include "ft6336g.h"

Scene* g_current_scene = nullptr;

void gui_set_current_scene(Scene* scene) {
    g_current_scene = scene;
    g_current_scene->redraw();
}

Scene* gui_get_current_scene() {
    return g_current_scene;
}

Scene::Scene() {}

void Scene::add_object(std::shared_ptr<SceneObject> obj) {
    m_objects.push_back(std::move(obj));
    if (g_current_scene == this) {
        obj->redraw();
    }
}

void Scene::redraw() {
    lcd_clear_foreground();
    for (auto obj : m_objects) {
        obj->redraw();
    }
}

SceneObject::SceneObject(Scene* parent, bool clickable) : m_parent(parent), m_clickable(clickable) {}

void SceneObject::trigger_redraw() {
    if (m_parent && g_current_scene == m_parent) {
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
             uint8_t color)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size),
      m_color(color) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             uint8_t color, const Font* font)
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

void Label::set_color(uint8_t color) {
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
    : SceneObject(parent, true), m_x(x), m_y(y), m_width(w), m_height(h) {}

void Button::redraw() {
    if (m_visible) {
        lcd_draw_rectangle(m_x, m_y, m_width, m_height, m_pressed ? 0x82 : 0xF2);
    }
}

void Button::handle_press(int x, int y) {

    if (x >= m_x && x <= m_x + m_width &&
        y >= m_y && y <= m_y + m_height) {
        // Inside button bounds
        m_pressed = true;
        trigger_redraw();
    }
}

void Button::handle_contact(int x, int y) {

    if (x < m_x || x > m_x + m_width ||
        y < m_y || y > m_y + m_height) {
        if (m_pressed) {
            // Movement from inside to outside button bounds
            m_pressed = false;
            trigger_redraw();
        }
    } else {
        if (!m_pressed) {
            // Movement from outside to inside button bounds
            m_pressed = true;
            trigger_redraw();
        }
    }
}

void Button::handle_release(int x, int y) {
    
    // If we were pressing inside the button bounds, register a click
    if (m_pressed) {
        m_pressed = false;
        trigger_redraw();
        
        printf("Button clicked at (%d, %d)\n", x, y);
    }
}

void gui_touch_irq() {
    ft6336g_irq();

    FT6336G_TouchEvent event = ft6336g_get_touch_event(0);
    if (event == FT6336G_TOUCH_EVENT_NONE) {
        return;
    }

    int x, y, weight;
    if (ft6336g_read_pos(&y, &x, &weight, 0) != STATUS_OK) {
        return;
    }

    if (event == FT6336G_TOUCH_EVENT_DOWN) {
        // Press event
        for (auto obj : g_current_scene->get_objects()) {
            if (obj->is_clickable()) {
                obj->handle_press(x, y);
            }
        }

        return;
    }

    if (event == FT6336G_TOUCH_EVENT_CONTACT) {
        // Contact event
        for (auto obj : g_current_scene->get_objects()) {
            if (obj->is_clickable()) {
                obj->handle_contact(x, y);
            }
        }

        return;
    }

    if (event == FT6336G_TOUCH_EVENT_UP) {
        // Release event
        for (auto obj : g_current_scene->get_objects()) {
            if (obj->is_clickable()) {
                obj->handle_release(x, y);
            }
        }

        return;
    }
}