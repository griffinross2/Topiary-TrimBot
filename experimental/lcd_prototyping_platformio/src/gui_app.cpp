#include "gui_app.h"

#include "lcd.h"
#include "gui.h"
#include "filesystem.h"
#include "timing.h"
#include "profiler.h"

// #include "images/test.h"
// #include "images/squares.h"
#include "images/splashscreen.h"
#include "images/blank.h"
#include "fonts/arial.h"

#include <algorithm>

struct {
    Scene scene;
    std::vector<FileInfo> file_list;
    int selected_file_index = 0;
    long long unsigned last_update_tick;
    std::shared_ptr<Rectangle> dialog_border;
    std::shared_ptr<Rectangle> dialog_bg;
    std::shared_ptr<Label> dialog_label;
    std::shared_ptr<Button> dialog_confirm_button;
    std::shared_ptr<Label> dialog_confirm_label;
    std::shared_ptr<Button> dialog_cancel_button;
    std::shared_ptr<Label> dialog_cancel_label;
} file_list_scene_ctx;

Status update_file_list() {
    PROFILER_ENTER();

    std::vector<FileInfo>& file_list = file_list_scene_ctx.file_list;
    Scene& scene = file_list_scene_ctx.scene;

    std::vector<FileInfo> new_file_list;
    Status status = filesystem_get_file_list(new_file_list);

    if (std::equal(file_list.begin(), file_list.end(), new_file_list.begin(),
                   new_file_list.end())) {
        // No change in file list, so we don't need to update
        PROFILER_EXIT();
        return STATUS_OK;
    }

    file_list = std::move(new_file_list);

    if (status != STATUS_OK) {
        PROFILER_EXIT();
        return status;
    }

    // Clear and re-add elements
    scene.clear_objects();

    for (size_t i = 0; i < file_list.size(); ++i) {
        auto button = std::make_shared<Button>(
            &scene, 0, LCD_HEIGHT - 48 - i * 48, LCD_WIDTH, 48);
        button->bg_off();
        button->set_on_click([scene, i](int x, int y) {
            file_list_scene_ctx.selected_file_index = i;
            file_list_scene_ctx.dialog_label->set_text(
                "Send " + file_list_scene_ctx.file_list[i].name + "?");
            file_list_scene_ctx.scene.set_dialog_active(true);
        });
        scene.add_object(button);

        auto name_label = std::make_shared<Label>(
            &scene, 10, LCD_HEIGHT - 48 - i * 48, file_list[i].name, 32);
        scene.add_object(name_label);

        auto div_line = std::make_shared<Rectangle>(
            &scene, 10, LCD_HEIGHT - 48 - i * 48 - 4, LCD_WIDTH - 20, 2,
            0x202020);
        scene.add_object(div_line);

        unsigned long long file_size = file_list[i].size;
        char size_str[32];
        if (file_size >= 1024 * 1024) {
            snprintf(size_str, sizeof(size_str), "%.2f MB",
                     file_size / (1024.0 * 1024.0));
        } else if (file_size >= 1024) {
            snprintf(size_str, sizeof(size_str), "%.2f KB", file_size / 1024.0);
        } else {
            snprintf(size_str, sizeof(size_str), "%lu B",
                     (unsigned long)file_size);
        }

        auto size_label = std::make_shared<Label>(
            &scene, LCD_WIDTH - 15, LCD_HEIGHT - 48 - i * 48, size_str, 32);
        size_label->set_alignment(LABEL_ALIGN_RIGHT);
        scene.add_object(size_label);
    }

    PROFILER_EXIT();
    return STATUS_OK;
};

Status load_file_list_scene() {
    // Dialog box for confirming a file
    Scene& scene = file_list_scene_ctx.scene;
    std::shared_ptr<Rectangle>& dialog_border =
        file_list_scene_ctx.dialog_border;
    std::shared_ptr<Rectangle>& dialog_bg = file_list_scene_ctx.dialog_bg;
    std::shared_ptr<Label>& dialog_label = file_list_scene_ctx.dialog_label;
    std::shared_ptr<Button>& dialog_confirm_button =
        file_list_scene_ctx.dialog_confirm_button;
    std::shared_ptr<Label>& dialog_confirm_label =
        file_list_scene_ctx.dialog_confirm_label;
    std::shared_ptr<Button>& dialog_cancel_button =
        file_list_scene_ctx.dialog_cancel_button;
    std::shared_ptr<Label>& dialog_cancel_label =
        file_list_scene_ctx.dialog_cancel_label;
    constexpr int dialog_width = 400;
    constexpr int dialog_height = 120;
    constexpr int dialog_border_thickness = 2;

    dialog_border = std::make_shared<Rectangle>(
        &scene, LCD_WIDTH / 2 - dialog_width / 2 - dialog_border_thickness,
        LCD_HEIGHT / 2 - dialog_height / 2 - dialog_border_thickness,
        dialog_width + dialog_border_thickness * 2,
        dialog_height + dialog_border_thickness * 2, 0x000000);
    dialog_bg =
        std::make_shared<Rectangle>(&scene, LCD_WIDTH / 2 - dialog_width / 2,
                                    LCD_HEIGHT / 2 - dialog_height / 2,
                                    dialog_width, dialog_height, 0xFFFFFF);

    scene.add_dialog_object(dialog_border);
    scene.add_dialog_object(dialog_bg);

    dialog_label = std::make_shared<Label>(&scene, LCD_WIDTH / 2,
                                           LCD_HEIGHT / 2 + 5, "", 32);
    dialog_label->set_alignment(LABEL_ALIGN_CENTER);

    scene.add_dialog_object(dialog_label);

    dialog_confirm_button = std::make_shared<Button>(
        &scene, LCD_WIDTH / 2 + 10, LCD_HEIGHT / 2 - dialog_height / 3,
        dialog_width / 3, dialog_height / 3);

    dialog_confirm_label = std::make_shared<Label>(
        &scene, LCD_WIDTH / 2 + dialog_width / 6 + 10,
        LCD_HEIGHT / 2 - dialog_height / 3 + 2, "OK", 32);
    dialog_confirm_label->set_alignment(LABEL_ALIGN_CENTER);

    dialog_confirm_button->set_on_click([](int x, int y) {
        printf("Clicked confirm for file: %s\n",
               file_list_scene_ctx
                   .file_list[file_list_scene_ctx.selected_file_index]
                   .name.c_str());
        file_list_scene_ctx.scene.set_dialog_active(false);
    });

    dialog_cancel_button =
        std::make_shared<Button>(&scene, LCD_WIDTH / 2 - dialog_width / 3 - 10,
                                 LCD_HEIGHT / 2 - dialog_height / 3,
                                 dialog_width / 3, dialog_height / 3);

    dialog_cancel_label = std::make_shared<Label>(
        &scene, LCD_WIDTH / 2 - dialog_width / 6 - 10,
        LCD_HEIGHT / 2 - dialog_height / 3 + 2, "Cancel", 32);
    dialog_cancel_label->set_alignment(LABEL_ALIGN_CENTER);

    dialog_cancel_button->set_on_click([](int x, int y) {
        printf("Clicked cancel for file: %s\n",
               file_list_scene_ctx
                   .file_list[file_list_scene_ctx.selected_file_index]
                   .name.c_str());
        file_list_scene_ctx.scene.set_dialog_active(false);
    });

    scene.add_dialog_object(dialog_confirm_button);
    scene.add_dialog_object(dialog_confirm_label);
    scene.add_dialog_object(dialog_cancel_button);
    scene.add_dialog_object(dialog_cancel_label);

    // Add in the file list
    update_file_list();

    gui_set_current_scene(&scene);

    return STATUS_OK;
}

Status gui_app_init() {
    lcd_set_background(BLANK);
    lcd_clear_foreground();
    lcd_swap_buffers();

    load_file_list_scene();

    return STATUS_OK;
}

void gui_app_task() {
    // Scene specific updates
    if (gui_get_current_scene() == &file_list_scene_ctx.scene) {
        if (get_tick_ms() - file_list_scene_ctx.last_update_tick >= 1000) {
            update_file_list();
            file_list_scene_ctx.last_update_tick = get_tick_ms();
        }
    }

    // Global update
    gui_update();
    gui_render();
}