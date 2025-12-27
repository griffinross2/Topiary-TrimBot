#include "gui.h"

#include "ft6336g.h"

#include <algorithm>

Scene* g_current_scene = nullptr;
TouchState g_touch_state = {false, 0, 0};

void gui_set_current_scene(Scene* scene) {
    g_current_scene = scene;
    g_current_scene->redraw(nullptr);
}

Scene* gui_get_current_scene() {
    return g_current_scene;
}

Scene::Scene() {}

void Scene::add_object(std::shared_ptr<SceneObject> obj) {
    m_objects.push_back(std::move(obj));
    if (g_current_scene == this) {
        redraw(obj.get());
    }
}

void Scene::redraw(SceneObject* obj) {
    // If obj is nullptr, redraw the entire scene
    if (obj == nullptr) {
        lcd_clear_foreground();
        for (auto& o : m_objects) {
            o->redraw();
        }
        m_draw_groups = create_draw_groups();
        return;
    }

    // Otherwise find the draw group containing the object
    // wanting to be redrawn
    for (auto& group : m_draw_groups) {
        auto it = std::find_if(
            group.objects.begin(),
            group.objects.end(),
            [obj](const std::shared_ptr<SceneObject>& o) {
                return o.get() == obj;
            }
        );

        if (it != group.objects.end()) {
            // Found a group containing the object

            // Clear the area of the group
            int clear_xl = std::min(std::max(0, group.bounds.xl), LCD_WIDTH - 1);
            int clear_xr = std::min(std::max(0, group.bounds.xr), LCD_WIDTH - 1);
            int clear_yb = std::min(std::max(0, group.bounds.yb), LCD_HEIGHT - 1);
            int clear_yt = std::min(std::max(0, group.bounds.yt), LCD_HEIGHT - 1);
            lcd_clear_area(
                clear_xl,
                clear_xr,
                clear_yb,
                clear_yt
            );

            // Redraw all objects in this group
            for (auto& o : group.objects) {
                o->redraw();
            }

            // Recalculate draw groups
            m_draw_groups = create_draw_groups();

            // Done
            return;
        }
    }

    // We should only be here if an object was just added
    // In this case, we are drawing it on top of everything
    // else so don't worry about clearing anything
    obj->redraw();
    m_draw_groups = create_draw_groups();
}

std::vector<DrawGroup> Scene::create_draw_groups() {
    // Start with all scene elements in individual groups
    // Then we will merge groups that overlap until none
    // overlap.

    std::vector<DrawGroup> groups;
    for (auto obj : m_objects) {
        DrawGroup group;
        group.objects.push_back(obj);
        group.bounds = obj->calc_bounds();
        groups.push_back(group);
    }

    bool overlapping = true;
    while (overlapping) {
        overlapping = false;

        for (size_t i = 0; i < groups.size(); i++) {
            for (size_t j = i + 1; j < groups.size(); j++) {
                // Check for overlap between groups
                Bounds& a = groups[i].bounds;
                Bounds& b = groups[j].bounds;

                bool overlap = (a.xl <= b.xr) && (a.xr >= b.xl) &&
                               (a.yb <= b.yt) && (a.yt >= b.yb);

                if (overlap) {
                    // Merge groups[j] into groups[i]
                    groups[i].objects.insert(
                        groups[i].objects.end(),
                        groups[j].objects.begin(),
                        groups[j].objects.end()
                    );

                    // Update bounds
                    groups[i].bounds.xl = std::min(a.xl, b.xl);
                    groups[i].bounds.xr = std::max(a.xr, b.xr);
                    groups[i].bounds.yb = std::min(a.yb, b.yb);
                    groups[i].bounds.yt = std::max(a.yt, b.yt);

                    // Remove groups[j]
                    groups.erase(groups.begin() + j);

                    // Check again
                    overlapping = true;
                    break;
                }
            }

            if (overlapping) {
                // Check again
                break;
            }
        }
    }

    return groups;
}

SceneObject::SceneObject(Scene* parent, bool clickable) : m_parent(parent), m_clickable(clickable) {}

void SceneObject::trigger_redraw() {
    if (m_parent && g_current_scene == m_parent) {
        m_parent->redraw(this);
    }
}

void SceneObject::set_visible(bool visible) {
    m_visible = visible;
    trigger_redraw();
}

Rectangle::Rectangle(Scene* parent, int x, int y, int w, int h, Color color)
    : SceneObject(parent), m_x(x), m_y(y), m_width(w), m_height(h), m_color(color) {}

void Rectangle::set_position(int x, int y) {
    m_x = x;
    m_y = y;
    trigger_redraw();
}

void Rectangle::set_size(int w, int h) {
    m_width = w;
    m_height = h;
    trigger_redraw();
}

void Rectangle::set_color(Color color) {
    m_color = color;
    trigger_redraw();
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

Bounds Rectangle::calc_bounds() {
    Bounds ret = {
        .xl = m_x,
        .xr = m_x + m_width,
        .yb = m_y,
        .yt = m_y + m_height,
    };

    return ret;
}

Label::Label(Scene* parent, int x, int y)
    : SceneObject(parent), m_x(x), m_y(y), m_text("") {}

Label::Label(Scene* parent, int x, int y, std::string text)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             Color color)
    : SceneObject(parent), m_x(x), m_y(y), m_text(text), m_size(size),
      m_color(color) {}

Label::Label(Scene* parent, int x, int y, std::string text, int size,
             Color color, const Font* font)
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

void Label::set_color(Color color) {
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

Bounds Label::calc_bounds() {
    // Calculate bounds based on text size and font metrics
    Bounds ret;
    ret.xl = m_x;
    ret.yb = m_y;
    ret.yt = m_y + m_size + 1;

    unsigned int text_width = 0;
    for (char ch : m_text) {
        text_width += m_font->glyphs[(uint8_t)ch]->advance * m_size / m_font->width;
    }

    ret.xr = ret.xl + text_width;

    return ret;
}

Button::Button(Scene* parent, int x, int y, int w, int h)
    : SceneObject(parent, true), m_x(x), m_y(y), m_width(w), m_height(h) {}

void Button::redraw() {
    if (m_visible) {
        int x = std::min(std::max(0, m_x), LCD_WIDTH - 1);
        int y = std::min(std::max(0, m_y), LCD_HEIGHT - 1);
        unsigned int w = std::min(m_width, LCD_WIDTH - x);
        unsigned int h = std::min(m_height, LCD_HEIGHT - y);
        lcd_draw_rectangle(x, y, w, h, m_pressed ? 0x82 : 0xF2);
    }
}

void Button::handle_press(int x, int y) {

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
            // Or start of a press
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

Bounds Button::calc_bounds() {
    Bounds ret = {
        .xl = m_x,
        .xr = m_x + m_width,
        .yb = m_y,
        .yt = m_y + m_height,
    };

    return ret;
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

void gui_update_loop() {
    gui_touch_update();
}