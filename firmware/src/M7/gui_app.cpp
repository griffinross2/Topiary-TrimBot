#include "gui_app.h"

#include "lcd.h"
#include "gui.h"
#include "filesystem.h"
#include "timing.h"
#include "profiler.h"
#include "file_sender.h"

// #include "images/test.h"
// #include "images/squares.h"
#include "images/splashscreen.h"
#include "images/blank.h"
#include "images/main.h"
#include "fonts/arial.h"
#include "graphics/arrow_up.h"
#include "graphics/arrow_down.h"

#include <algorithm>

static int s_init_status = STATUS_ERROR;

/************************/
/* STARTUP SPLASHSCREEN */
/************************/

static Scene s_splash_screen_scene;

static Status load_splash_screen_scene() {
    lcd_set_background(SPLASHSCREEN);

    gui_set_current_scene(&s_splash_screen_scene);

    return STATUS_OK;
}

/*************************/
/* FILE SELECTION SCREEN */
/*************************/

#define MAX_FILES_DISPLAYED 6

struct {
    Scene scene;
    std::vector<FileInfo> file_list;
    size_t selected_file_index = 0;
    size_t file_list_start_index = 0;
    size_t file_list_last_size = 0;
    long long unsigned last_update_tick;
    Button file_button[MAX_FILES_DISPLAYED];
    Label file_label[MAX_FILES_DISPLAYED];
    Label file_size_label[MAX_FILES_DISPLAYED];
    Rectangle file_divider[MAX_FILES_DISPLAYED];
    Button file_scroll_up_button;
    Button file_scroll_down_button;
    GraphicsObject file_scroll_up_graphic;
    GraphicsObject file_scroll_down_graphic;
    Rectangle dialog_border;
    Rectangle dialog_bg;
    Label dialog_label;
    Button dialog_confirm_button;
    Label dialog_confirm_label;
    Button dialog_cancel_button;
    Label dialog_cancel_label;
} file_list_scene_ctx;

static Status update_file_list(bool force_update = false) {
    PROFILER_ENTER();

    std::vector<FileInfo>& file_list = file_list_scene_ctx.file_list;
    Scene& scene = file_list_scene_ctx.scene;

    std::vector<FileInfo> new_file_list;
    Status status = filesystem_get_file_list(new_file_list);

    // If the size changed, reset the scroll
    if (new_file_list.size() != file_list_scene_ctx.file_list_last_size) {
        file_list_scene_ctx.file_list_last_size = new_file_list.size();
        file_list_scene_ctx.file_list_start_index = 0;
    }

    // If the file list has become empty, close the dialog
    if (new_file_list.empty()) {
        file_list_scene_ctx.scene.set_dialog_active(false);
    }

    if (!force_update &&
        std::equal(file_list.begin(), file_list.end(), new_file_list.begin(),
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

    // Readd static elements
    scene.add_object(&file_list_scene_ctx.file_scroll_up_button);
    scene.add_object(&file_list_scene_ctx.file_scroll_down_button);
    scene.add_object(&file_list_scene_ctx.file_scroll_up_graphic);
    scene.add_object(&file_list_scene_ctx.file_scroll_down_graphic);

    for (size_t i = 0;
         i <
         std::min(file_list.size() - file_list_scene_ctx.file_list_start_index,
                  static_cast<size_t>(MAX_FILES_DISPLAYED));
         ++i) {
        size_t file_idx = file_list_scene_ctx.file_list_start_index + i;

        file_list_scene_ctx.file_button[i] = Button(
            &scene, 40, WINDOW_HEIGHT - 48 - i * 48, WINDOW_WIDTH - 130, 48);
        file_list_scene_ctx.file_button[i].bg_off();
        file_list_scene_ctx.file_button[i].set_on_click([scene, i, file_idx](
                                                            int x, int y) {
            file_list_scene_ctx.selected_file_index = file_idx;
            file_list_scene_ctx.dialog_label.set_text(
                "Send " + file_list_scene_ctx.file_list[file_idx].name + "?");
            file_list_scene_ctx.scene.set_dialog_active(true);
        });
        scene.add_object(&file_list_scene_ctx.file_button[i]);

        file_list_scene_ctx.file_label[i] =
            Label(&scene, 50, WINDOW_HEIGHT - 48 - i * 48,
                  file_list[file_idx].name, 32);
        scene.add_object(&file_list_scene_ctx.file_label[i]);

        file_list_scene_ctx.file_divider[i] =
            Rectangle(&scene, 50, WINDOW_HEIGHT - 48 - i * 48 - 4,
                      WINDOW_WIDTH - 150, 2, 0xFA);
        scene.add_object(&file_list_scene_ctx.file_divider[i]);

        unsigned long long file_size = file_list[file_idx].size;
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

        file_list_scene_ctx.file_size_label[i] =
            Label(&scene, WINDOW_WIDTH - 105, WINDOW_HEIGHT - 48 - i * 48,
                  size_str, 32);
        file_list_scene_ctx.file_size_label[i].set_alignment(LABEL_ALIGN_RIGHT);
        scene.add_object(&file_list_scene_ctx.file_size_label[i]);
    }

    PROFILER_EXIT();
    return STATUS_OK;
};

static Status load_file_list_scene() {
    // Dialog box for confirming a file
    Scene& scene = file_list_scene_ctx.scene;
    Rectangle* dialog_border = &file_list_scene_ctx.dialog_border;
    Rectangle* dialog_bg = &file_list_scene_ctx.dialog_bg;
    Label* dialog_label = &file_list_scene_ctx.dialog_label;
    Button* dialog_confirm_button = &file_list_scene_ctx.dialog_confirm_button;
    Label* dialog_confirm_label = &file_list_scene_ctx.dialog_confirm_label;
    Button* dialog_cancel_button = &file_list_scene_ctx.dialog_cancel_button;
    Label* dialog_cancel_label = &file_list_scene_ctx.dialog_cancel_label;
    constexpr int dialog_width = 400;
    constexpr int dialog_height = 120;
    constexpr int dialog_border_thickness = 2;

    *dialog_border = Rectangle(
        &scene, WINDOW_WIDTH / 2 - dialog_width / 2 - dialog_border_thickness,
        WINDOW_HEIGHT / 2 - dialog_height / 2 - dialog_border_thickness,
        dialog_width + dialog_border_thickness * 2,
        dialog_height + dialog_border_thickness * 2, 0xF1);
    *dialog_bg = Rectangle(&scene, WINDOW_WIDTH / 2 - dialog_width / 2,
                           WINDOW_HEIGHT / 2 - dialog_height / 2, dialog_width,
                           dialog_height, 0xF0);

    scene.add_dialog_object(dialog_border);
    scene.add_dialog_object(dialog_bg);

    *dialog_label =
        Label(&scene, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 5, "", 32);
    dialog_label->set_alignment(LABEL_ALIGN_CENTER);

    scene.add_dialog_object(dialog_label);

    *dialog_confirm_button = Button(&scene, WINDOW_WIDTH / 2 + 10,
                                    WINDOW_HEIGHT / 2 - dialog_height / 3,
                                    dialog_width / 3, dialog_height / 3);

    *dialog_confirm_label =
        Label(&scene, WINDOW_WIDTH / 2 + dialog_width / 6 + 10,
              WINDOW_HEIGHT / 2 - dialog_height / 3 + 2, "OK", 32);
    dialog_confirm_label->set_alignment(LABEL_ALIGN_CENTER);

    dialog_confirm_button->set_on_click([](int x, int y) {
        // Send file if the index is still valid (card wasn't removed)
        if (file_list_scene_ctx.selected_file_index <
            file_list_scene_ctx.file_list.size()) {
            const FileInfo& selected_file =
                file_list_scene_ctx
                    .file_list[file_list_scene_ctx.selected_file_index];
            file_sender_send_file(selected_file.name.c_str());
            TRACE_PRINTF("Sending file: %s\n", selected_file.name.c_str());
        }

        file_list_scene_ctx.scene.set_dialog_active(false);
    });

    *dialog_cancel_button =
        Button(&scene, WINDOW_WIDTH / 2 - dialog_width / 3 - 10,
               WINDOW_HEIGHT / 2 - dialog_height / 3, dialog_width / 3,
               dialog_height / 3);

    *dialog_cancel_label =
        Label(&scene, WINDOW_WIDTH / 2 - dialog_width / 6 - 10,
              WINDOW_HEIGHT / 2 - dialog_height / 3 + 2, "Cancel", 32);
    dialog_cancel_label->set_alignment(LABEL_ALIGN_CENTER);

    dialog_cancel_button->set_on_click([](int x, int y) {
        file_list_scene_ctx.scene.set_dialog_active(false);
    });

    scene.add_dialog_object(dialog_confirm_button);
    scene.add_dialog_object(dialog_confirm_label);
    scene.add_dialog_object(dialog_cancel_button);
    scene.add_dialog_object(dialog_cancel_label);

    // Static file list components
    file_list_scene_ctx.file_scroll_up_button =
        Button(&scene, WINDOW_WIDTH - 80, WINDOW_HEIGHT - 40 - 10, 40, 40);
    file_list_scene_ctx.file_scroll_up_button.set_on_click([](int x, int y) {
        if (file_list_scene_ctx.file_list_start_index > 0) {
            file_list_scene_ctx.file_list_start_index--;
            update_file_list(true);
        }
    });

    file_list_scene_ctx.file_scroll_down_button =
        Button(&scene, WINDOW_WIDTH - 80, 10, 40, 40);
    file_list_scene_ctx.file_scroll_down_button.set_on_click([](int x, int y) {
        if (file_list_scene_ctx.file_list_start_index + MAX_FILES_DISPLAYED <
            file_list_scene_ctx.file_list.size()) {
            file_list_scene_ctx.file_list_start_index++;
            update_file_list(true);
        }
    });

    file_list_scene_ctx.file_scroll_up_graphic = GraphicsObject(
        &scene, ARROW_UP, WINDOW_WIDTH - 80, WINDOW_HEIGHT - 40 - 10);

    file_list_scene_ctx.file_scroll_down_graphic =
        GraphicsObject(&scene, ARROW_DOWN, WINDOW_WIDTH - 80, 10);

    // Add in the file list
    update_file_list();

    lcd_set_background(MAIN);
    gui_set_current_scene(&scene);

    return STATUS_OK;
}

Status gui_app_init() {
    lcd_set_background(BLANK);
    lcd_clear_foreground();
    lcd_swap_buffers();

    load_splash_screen_scene();

    return STATUS_OK;
}

void gui_app_init_status(int status) {
    s_init_status = status;
}

void gui_app_task() {
    PROFILER_ENTER();

    static uint32_t last_fps_check = HAL_GetTick();
    static int num_frames = 0;

    // Scene specific updates

    // Splash screen
    if (gui_get_current_scene() == &s_splash_screen_scene) {
        if (get_tick_ms() >= 3000) {
            if (s_init_status != STATUS_OK) {
                // If initialization failed, just show a blank screen
                lcd_set_background(BLANK);
                return;
            } else {
                // Load the file list scene after the splash screen
                load_file_list_scene();
            }
        }
    }

    // File list
    if (gui_get_current_scene() == &file_list_scene_ctx.scene) {
        if (get_tick_ms() - file_list_scene_ctx.last_update_tick >= 1000) {
            update_file_list();
            file_list_scene_ctx.last_update_tick = get_tick_ms();
        }
    }

    // Global update
    gui_update();
    gui_render();

    num_frames++;

    // Print framerate
    if (HAL_GetTick() - last_fps_check >= 5000) {
        float fps = 1000.0f * num_frames / (HAL_GetTick() - last_fps_check);
        printf("FPS: %.2f\n", fps);
        last_fps_check = HAL_GetTick();
        num_frames = 0;
    }

    PROFILER_EXIT();
}