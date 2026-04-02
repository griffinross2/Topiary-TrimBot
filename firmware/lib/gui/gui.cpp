#include "gui.h"

#include "tsc2013/tsc2013.h"
// #include "profiler.h"
#include "timing.h"

#include <algorithm>

static Scene* s_current_scene = nullptr;
static TouchState s_touch_state = {false, 0, 0};
static uint32_t s_last_refresh = 0;

void gui_set_current_scene(Scene* scene) {
    s_current_scene = scene;
    s_current_scene->redraw();
}

Scene* gui_get_current_scene() {
    return s_current_scene;
}

Scene::Scene() {}

Scene::Scene(Color background_color) {
    m_background_color = background_color;
}

void Scene::add_object(std::shared_ptr<SceneObject> obj) {
    m_objects.push_back(std::move(obj));
    if (s_current_scene == this) {
        redraw();
    }
}

void Scene::add_dialog_object(std::shared_ptr<SceneObject> obj) {
    m_dialog_objects.push_back(std::move(obj));
    if (s_current_scene == this) {
        redraw();
    }
}

void Scene::redraw() {
    if (m_background_color != 0xF0) {
        lcd_draw_rectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                           m_background_color);
    } else {
        lcd_clear_foreground();
    }

    for (auto& o : m_objects) {
        o->redraw();
    }

    if (m_dialog_active) {
        for (auto& o : m_dialog_objects) {
            o->redraw();
        }
    }

    lcd_swap_buffers();
}

SceneObject::SceneObject(Scene* parent, bool clickable)
    : m_parent(parent), m_clickable(clickable) {}

void SceneObject::set_visible(bool visible) {
    m_visible = visible;
}

Rectangle::Rectangle(Scene* parent, int x, int y, int w, int h,
                     Color color)
    : SceneObject(parent), m_x(x), m_y(y), m_width(w), m_height(h), m_color(color) {
}

void Rectangle::set_position(int x, int y) {
    m_x = x;
    m_y = y;
}

void Rectangle::set_size(int w, int h) {
    m_width = w;
    m_height = h;
}

void Rectangle::set_color(Color color) {
    m_color = color;
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
             Color color)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size), m_color(color) {
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             Color color, const Font* font)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size), m_color(color),
      m_font(font) {
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

void Label::set_position(int x, int y) {
    m_x = x;
    m_y = y;
}

void Label::set_text(std::string text) {
    m_text = text;
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

void Label::set_color(Color color) {
    m_color = color;
}

void Label::set_font(const Font* font) {
    m_font = font;
    m_text_width = lcd_get_text_width(m_font, m_text.c_str(), m_size);
}

void Label::redraw() {
    if (m_visible) {
        if (m_alignment == LABEL_ALIGN_RIGHT) {
            lcd_draw_text(m_font, m_text.c_str(), m_x - m_text_width, m_y,
                          m_size, m_color);
        } else if (m_alignment == LABEL_ALIGN_CENTER) {
            lcd_draw_text(m_font, m_text.c_str(), m_x - m_text_width / 2, m_y,
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
        if (m_background_on && !m_pressed) {
            lcd_draw_rectangle(x, y, w, h, 0xF8);
        } else if (m_pressed) {
            lcd_draw_rectangle(x, y, w, h, 0xF9);
        }
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

        if (m_on_click) {
            m_on_click(x, y);
        }
    }
}

void gui_touch_poll() {
    if (tsc2013_is_touched()) {
        uint16_t x, y, z;
        if (tsc2013_is_data_ready() && tsc2013_read_touch(&x, &y, &z) == STATUS_OK) {
            s_touch_state.pressed = true;
            s_touch_state.x = x - (LCD_WIDTH - LTDC_WINDOW_WIDTH) / 2;
            s_touch_state.y = y - (LCD_HEIGHT - LTDC_WINDOW_HEIGHT) / 2;
        }
    } else {
        s_touch_state.pressed = false;
    }
}

void gui_touch_update() {
    int x = s_touch_state.x;
    int y = s_touch_state.y;
    if (s_touch_state.pressed) {
        if (s_current_scene->is_dialog_active()) {
            for (auto obj : s_current_scene->get_dialog_objects()) {
                if (obj->is_clickable() && obj->is_visible()) {
                    obj->handle_press(x, y);
                }
            }
        } else {
            for (auto obj : s_current_scene->get_objects()) {
                if (obj->is_clickable() && obj->is_visible()) {
                    obj->handle_press(x, y);
                }
            }
        }
    } else {
        if (s_current_scene->is_dialog_active()) {
            for (auto obj : s_current_scene->get_dialog_objects()) {
                if (obj->is_clickable() && obj->is_visible()) {
                    obj->handle_release(x, y);
                }
            }
        } else {
            for (auto obj : s_current_scene->get_objects()) {
                if (obj->is_clickable() && obj->is_visible()) {
                    obj->handle_release(x, y);
                }
            }
        }
    }
}

void gui_update() {
    // PROFILER_ENTER();

    gui_touch_poll();
    gui_touch_update();
    // printf("Touch state: %s at (%d, %d)\n", s_touch_state.pressed ? "Pressed" : "Released",
    //        s_touch_state.x, s_touch_state.y);

    // PROFILER_EXIT();
}

void gui_render(unsigned int target_fps) {
    // PROFILER_ENTER();

    if (get_tick_ms() - s_last_refresh < 1000 / target_fps) {
        // PROFILER_EXIT();
        return;
    }
    s_last_refresh = get_tick_ms();

    if (s_current_scene) {
        s_current_scene->redraw();
    }

    // PROFILER_EXIT();
}