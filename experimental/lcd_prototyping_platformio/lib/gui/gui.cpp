#include "gui.h"

#include "ft6336g.h"

#include <algorithm>

Scene* g_current_scene = nullptr;
TouchState g_touch_state = {false, 0, 0};

void gui_set_current_scene(Scene* scene) {
    g_current_scene = scene;
    g_current_scene->redraw();
}

Scene* gui_get_current_scene() {
    return g_current_scene;
}

Scene::Scene() {}

Scene::Scene(ColorRGB888 background_color) {
    m_background_color = rgb888_to_rgb565(background_color);
}

void Scene::add_object(std::shared_ptr<SceneObject> obj) {
    m_objects.push_back(std::move(obj));
    if (g_current_scene == this) {
        redraw();
    }
}

void Scene::redraw() {
    if (m_background_color != 0xFFFF) {
        lcd_draw_rectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                           m_background_color);
    } else {
        lcd_clear_foreground();
    }

    for (auto& o : m_objects) {
        o->redraw();
    }

    lcd_swap_buffers();
}

SceneObject::SceneObject(Scene* parent, bool clickable)
    : m_parent(parent), m_clickable(clickable) {}

void SceneObject::set_visible(bool visible) {
    m_visible = visible;
}

Rectangle::Rectangle(Scene* parent, int x, int y, int w, int h,
                     ColorRGB888 color)
    : SceneObject(parent), m_x(x), m_y(y), m_width(w), m_height(h) {
    m_color = rgb888_to_rgb565(color);
}

void Rectangle::set_position(int x, int y) {
    m_x = x;
    m_y = y;
}

void Rectangle::set_size(int w, int h) {
    m_width = w;
    m_height = h;
}

void Rectangle::set_color(ColorRGB888 color) {
    m_color = rgb888_to_rgb565(color);
}

void Rectangle::redraw() {
    if (m_visible) {
        int x = std::min(std::max(0, m_x), LCD_WIDTH - 1);
        int y = std::min(std::max(0, m_y), LCD_HEIGHT - 1);
        unsigned int w = std::min(m_width, LCD_WIDTH - x);
        unsigned int h = std::min(m_height, LCD_HEIGHT - y);
        lcd_draw_rectangle(x, y, w, h, m_color);
    }
}

Label::Label(Scene* parent, int x, int y)
    : SceneObject(parent), m_x(x), m_y(y), m_text("") {}

Label::Label(Scene* parent, int x, int y, std::string text)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text) {
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

Label::Label(Scene* parent, int x, int y, std::string text, int size)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size) {
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             ColorRGB888 color)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size) {
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
    m_color = rgb888_to_rgb565(color);
}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             ColorRGB888 color, const Font* font)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size),
      m_font(font) {
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
    m_color = rgb888_to_rgb565(color);
}

void Label::set_position(int x, int y) {
    m_x = x;
    m_y = y;
}

void Label::set_text(std::string text) {
    m_text = text;
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

void Label::set_color(ColorRGB888 color) {
    m_color = rgb888_to_rgb565(color);
}

void Label::set_font(const Font* font) {
    m_font = font;
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

void Label::redraw() {
    if (m_visible) {
        if (m_right_align) {
            lcd_draw_text(m_font, m_text.c_str(), m_x - m_text_width, m_y,
                          m_size, m_color);
        } else {
            lcd_draw_text(m_font, m_text.c_str(), m_x, m_y, m_size, m_color);
        }
    }
}

Button::Button(Scene* parent, int x, int y, int w, int h)
    : SceneObject(parent, true), m_x(x), m_y(y), m_width(w), m_height(h) {}

void Button::redraw() {
    if (m_visible) {
        int x = std::min(std::max(0, m_x), LCD_WIDTH - 1);
        int y = std::min(std::max(0, m_y), LCD_HEIGHT - 1);
        unsigned int w = std::min(m_width, LCD_WIDTH - x);
        unsigned int h = std::min(m_height, LCD_HEIGHT - y);
        lcd_draw_rectangle(x, y, w, h,
                           m_pressed ? LITERAL_RGB888_TO_RGB565(0xBBBBBB)
                                     : LITERAL_RGB888_TO_RGB565(0x888888));
    }
}

void Button::handle_press(int x, int y) {
    if (x < m_x || x > m_x + m_width || y < m_y || y > m_y + m_height) {
        if (m_pressed) {
            // Movement from inside to outside button bounds
            m_pressed = false;
        }
    } else {
        if (!m_pressed) {
            // Movement from outside to inside button bounds
            // Or start of a press
            m_pressed = true;
        }
    }
}

void Button::handle_release(int x, int y) {
    // If we were pressing inside the button bounds, register a click
    if (m_pressed) {
        m_pressed = false;

        printf("Button clicked at (%d, %d)\n", x, y);
    }
}

void gui_touch_update() {
    int x = g_touch_state.x;
    int y = g_touch_state.y;
    if (g_touch_state.pressed) {
        for (auto obj : g_current_scene->get_objects()) {
            if (obj->is_clickable()) {
                obj->handle_press(x, y);
            }
        }
    } else {
        for (auto obj : g_current_scene->get_objects()) {
            if (obj->is_clickable()) {
                obj->handle_release(x, y);
            }
        }
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
        g_touch_state.pressed = true;
        g_touch_state.x = x;
        g_touch_state.y = y;

        return;
    }

    if (event == FT6336G_TOUCH_EVENT_CONTACT) {
        // Contact event
        g_touch_state.pressed = true;
        g_touch_state.x = x;
        g_touch_state.y = y;

        return;
    }

    if (event == FT6336G_TOUCH_EVENT_UP) {
        // Release event
        g_touch_state.pressed = false;
        g_touch_state.x = x;
        g_touch_state.y = y;

        return;
    }
}

void gui_update() {
    gui_touch_update();
}

void gui_render() {
    if (g_current_scene) {
        g_current_scene->redraw();
    }
}