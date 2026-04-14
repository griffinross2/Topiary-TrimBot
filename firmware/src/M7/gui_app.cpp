#include "gui_app.h"

#include "lcd.h"
#include "gui.h"
#include "filesystem.h"
#include "timing.h"
#include "profiler.h"
#include "file_sender.h"

#include "images/splashscreen.h"
#include "images/blank.h"
#include "images/main.h"
#include "images/file_send_error.h"
#include "images/begin_scanning.h"
#include "images/insert_card.h"
#include "fonts/arial.h"
#include "graphics/arrow_up.h"
#include "graphics/arrow_down.h"
#include "graphics/loading0.h"
#include "graphics/loading1.h"
#include "graphics/loading2.h"
#include "graphics/loading3.h"
#include "graphics/loading4.h"
#include "graphics/loading5.h"
#include "graphics/loading6.h"
#include "graphics/loading7.h"

#include <algorithm>

static int s_init_status = STATUS_ERROR;

// Declare scene helpers
static Status load_splash_screen_scene();
static Status load_file_list_scene();
static Status update_file_list(bool force_update = false);
static Status load_sending_file_scene();
static Status update_sending_file_scene();
static Status load_error_sending_file_scene();
static Status load_begin_scanning_scene();
static Status load_insert_card_scene();
static Status update_insert_card_scene();

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

static Status update_file_list(bool force_update) {
    PROFILER_ENTER();

    // If the SD card was removed close the dialog and switch to the insert card
    // scene
    if (!filesystem_is_card_inserted()) {
        file_list_scene_ctx.scene.set_dialog_active(false);
        load_insert_card_scene();
        PROFILER_EXIT();
        return STATUS_OK;
    }

    // Need to check if the file list changed or we scrolled
    // then update the text and or number of files displayed
    // Force update is true when the scroll button causes an update

    std::vector<FileInfo>& file_list = file_list_scene_ctx.file_list;

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

    for (size_t i = 0; i < MAX_FILES_DISPLAYED; ++i) {
        // Check if this line should display a file or if we are out
        bool line_visible =
            i < file_list.size() - file_list_scene_ctx.file_list_start_index;

        if (!line_visible) {
            file_list_scene_ctx.file_button[i].set_visible(false);
            file_list_scene_ctx.file_label[i].set_visible(false);
            file_list_scene_ctx.file_size_label[i].set_visible(false);
            file_list_scene_ctx.file_divider[i].set_visible(false);
            continue;
        }

        size_t file_idx = file_list_scene_ctx.file_list_start_index + i;
        file_list_scene_ctx.file_label[i].set_text(file_list[file_idx].name);

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

        file_list_scene_ctx.file_size_label[i].set_text(size_str);

        file_list_scene_ctx.file_button[i].set_visible(true);
        file_list_scene_ctx.file_label[i].set_visible(true);
        file_list_scene_ctx.file_size_label[i].set_visible(true);
        file_list_scene_ctx.file_divider[i].set_visible(true);
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

    *dialog_label =
        Label(&scene, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 5, "", 32);
    dialog_label->set_alignment(LABEL_ALIGN_CENTER);

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

            // Go to file sending screen
            load_sending_file_scene();
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

    scene.add_dialog_object(dialog_border);
    scene.add_dialog_object(dialog_bg);
    scene.add_dialog_object(dialog_label);
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
    file_list_scene_ctx.file_scroll_up_button.bg_off();

    file_list_scene_ctx.file_scroll_down_button =
        Button(&scene, WINDOW_WIDTH - 80, 10, 40, 40);
    file_list_scene_ctx.file_scroll_down_button.set_on_click([](int x, int y) {
        if (file_list_scene_ctx.file_list_start_index + MAX_FILES_DISPLAYED <
            file_list_scene_ctx.file_list.size()) {
            file_list_scene_ctx.file_list_start_index++;
            update_file_list(true);
        }
    });
    file_list_scene_ctx.file_scroll_down_button.bg_off();

    file_list_scene_ctx.file_scroll_up_graphic = GraphicsObject(
        &scene, ARROW_UP, WINDOW_WIDTH - 80, WINDOW_HEIGHT - 40 - 10);

    file_list_scene_ctx.file_scroll_down_graphic =
        GraphicsObject(&scene, ARROW_DOWN, WINDOW_WIDTH - 80, 10);

    scene.add_object(&file_list_scene_ctx.file_scroll_up_button);
    scene.add_object(&file_list_scene_ctx.file_scroll_down_button);
    scene.add_object(&file_list_scene_ctx.file_scroll_up_graphic);
    scene.add_object(&file_list_scene_ctx.file_scroll_down_graphic);

    // Add in the file list
    for (size_t i = 0; i < MAX_FILES_DISPLAYED; ++i) {
        file_list_scene_ctx.file_button[i] = Button(
            &scene, 40, WINDOW_HEIGHT - 48 - i * 48, WINDOW_WIDTH - 130, 48);
        file_list_scene_ctx.file_button[i].bg_off();
        file_list_scene_ctx.file_button[i].set_on_click([scene, i](int x,
                                                                   int y) {
            size_t file_idx = file_list_scene_ctx.file_list_start_index + i;
            file_list_scene_ctx.selected_file_index = file_idx;
            file_list_scene_ctx.dialog_label.set_text(
                "Send " + file_list_scene_ctx.file_list[file_idx].name + "?");
            file_list_scene_ctx.scene.set_dialog_active(true);
        });

        file_list_scene_ctx.file_label[i] =
            Label(&scene, 50, WINDOW_HEIGHT - 48 - i * 48, "", 32);

        file_list_scene_ctx.file_divider[i] =
            Rectangle(&scene, 50, WINDOW_HEIGHT - 48 - i * 48 - 4,
                      WINDOW_WIDTH - 150, 2, 0xFA);

        file_list_scene_ctx.file_size_label[i] = Label(
            &scene, WINDOW_WIDTH - 105, WINDOW_HEIGHT - 48 - i * 48, "", 32);
        file_list_scene_ctx.file_size_label[i].set_alignment(LABEL_ALIGN_RIGHT);

        // Invisible by default
        file_list_scene_ctx.file_button[i].set_visible(false);
        file_list_scene_ctx.file_label[i].set_visible(false);
        file_list_scene_ctx.file_divider[i].set_visible(false);
        file_list_scene_ctx.file_size_label[i].set_visible(false);

        scene.add_object(&file_list_scene_ctx.file_button[i]);
        scene.add_object(&file_list_scene_ctx.file_label[i]);
        scene.add_object(&file_list_scene_ctx.file_divider[i]);
        scene.add_object(&file_list_scene_ctx.file_size_label[i]);
    }

    // Update the list objects
    update_file_list();

    lcd_set_background(MAIN);
    gui_set_current_scene(&scene);

    return STATUS_OK;
}

/***********************/
/* SENDING FILE SCREEN */
/***********************/

struct {
    Scene scene;
    Label status_label;
    GraphicsObject spinner;
    size_t animation_idx = 0;
    long long unsigned last_update_tick;
} sending_file_scene_ctx;

static Status load_sending_file_scene() {
    Scene& scene = sending_file_scene_ctx.scene;
    Label& status_label = sending_file_scene_ctx.status_label;
    GraphicsObject& spinner = sending_file_scene_ctx.spinner;

    status_label = Label(&scene, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 20,
                         "Sending file...", 32);
    status_label.set_alignment(LABEL_ALIGN_CENTER);

    spinner =
        GraphicsObject(&scene, LOADING0, WINDOW_WIDTH / 2 - LOADING0.width / 2,
                       WINDOW_HEIGHT / 2 - 20 - LOADING0.height);

    scene.add_object(&spinner);
    scene.add_object(&status_label);

    lcd_set_background(MAIN);
    gui_set_current_scene(&scene);

    return STATUS_OK;
}

static Status update_sending_file_scene() {
    // Go to error screen if the file sender is in the error state
    if (file_sender_get_status() == FILE_SENDER_STATUS_ERROR) {
        file_sender_reset();
        load_error_sending_file_scene();
        return STATUS_OK;
    }

    // Go to the scanning confirmation screen if the file sender is done
    if (file_sender_get_status() == FILE_SENDER_STATUS_SUCCESS) {
        file_sender_reset();
        load_begin_scanning_scene();
        return STATUS_OK;
    }

    // Looping 8-frame animation
    sending_file_scene_ctx.animation_idx =
        (sending_file_scene_ctx.animation_idx + 1) % 8;

    switch (sending_file_scene_ctx.animation_idx) {
        case 0:
            sending_file_scene_ctx.spinner.set_graphics(LOADING0);
            break;
        case 1:
            sending_file_scene_ctx.spinner.set_graphics(LOADING1);
            break;
        case 2:
            sending_file_scene_ctx.spinner.set_graphics(LOADING2);
            break;
        case 3:
            sending_file_scene_ctx.spinner.set_graphics(LOADING3);
            break;
        case 4:
            sending_file_scene_ctx.spinner.set_graphics(LOADING4);
            break;
        case 5:
            sending_file_scene_ctx.spinner.set_graphics(LOADING5);
            break;
        case 6:
            sending_file_scene_ctx.spinner.set_graphics(LOADING6);
            break;
        case 7:
            sending_file_scene_ctx.spinner.set_graphics(LOADING7);
            break;
        default:
            break;
    }

    return STATUS_OK;
}

/*****************************/
/* ERROR SENDING FILE SCREEN */
/*****************************/

struct {
    Scene scene;
    Button back_button;
    Label back_label;
} error_sending_file_scene_ctx;

constexpr unsigned int BACK_BUTTON_WIDTH = 150;
constexpr unsigned int BACK_BUTTON_HEIGHT = 60;
constexpr unsigned int BACK_TEXT_SIZE = 32;

static Status load_error_sending_file_scene() {
    Scene& scene = error_sending_file_scene_ctx.scene;
    Button& back_button = error_sending_file_scene_ctx.back_button;
    Label& back_label = error_sending_file_scene_ctx.back_label;

    back_button = Button(&scene, WINDOW_WIDTH / 2 - BACK_BUTTON_WIDTH / 2, 0,
                         BACK_BUTTON_WIDTH, BACK_BUTTON_HEIGHT);
    back_button.set_on_click([](int x, int y) {
        load_file_list_scene();
        update_file_list(true);
    });

    back_label = Label(&scene, WINDOW_WIDTH / 2,
                       BACK_BUTTON_HEIGHT / 2 - BACK_TEXT_SIZE / 2, "Back",
                       BACK_TEXT_SIZE);
    back_label.set_alignment(LABEL_ALIGN_CENTER);

    scene.add_object(&back_button);
    scene.add_object(&back_label);

    lcd_set_background(FILE_SEND_ERROR);
    gui_set_current_scene(&scene);

    return STATUS_OK;
}

/*************************/
/* BEGIN SCANNING SCREEN */
/*************************/

struct {
    Scene scene;
    Button cancel_button;
    Label cancel_label;
    Button confirm_button;
    Label confirm_label;
} begin_scanning_scene_ctx;

constexpr unsigned int BEGIN_SCANNING_BUTTON_WIDTH = 150;
constexpr unsigned int BEGIN_SCANNING_BUTTON_HEIGHT = 60;
constexpr unsigned int BEGIN_SCANNING_TEXT_SIZE = 32;

static Status load_begin_scanning_scene() {
    Scene& scene = begin_scanning_scene_ctx.scene;
    Button& cancel_button = begin_scanning_scene_ctx.cancel_button;
    Label& cancel_label = begin_scanning_scene_ctx.cancel_label;
    Button& confirm_button = begin_scanning_scene_ctx.confirm_button;
    Label& confirm_label = begin_scanning_scene_ctx.confirm_label;

    cancel_button =
        Button(&scene, WINDOW_WIDTH / 2 - BEGIN_SCANNING_BUTTON_WIDTH - 10, 0,
               BEGIN_SCANNING_BUTTON_WIDTH, BEGIN_SCANNING_BUTTON_HEIGHT);
    cancel_button.set_on_click([](int x, int y) {
        load_file_list_scene();
        update_file_list(true);
    });

    cancel_label =
        Label(&scene, WINDOW_WIDTH / 2 - BEGIN_SCANNING_BUTTON_WIDTH / 2 - 10,
              BEGIN_SCANNING_BUTTON_HEIGHT / 2 - BEGIN_SCANNING_TEXT_SIZE / 2,
              "Cancel", BEGIN_SCANNING_TEXT_SIZE);
    cancel_label.set_alignment(LABEL_ALIGN_CENTER);

    confirm_button =
        Button(&scene, WINDOW_WIDTH / 2 + 10, 0, BEGIN_SCANNING_BUTTON_WIDTH,
               BEGIN_SCANNING_BUTTON_HEIGHT);
    confirm_button.set_on_click([](int x, int y) {
        // TODO: Change to start scanning
        load_file_list_scene();
        update_file_list(true);
    });

    confirm_label =
        Label(&scene, WINDOW_WIDTH / 2 + BEGIN_SCANNING_BUTTON_WIDTH / 2 + 10,
              BEGIN_SCANNING_BUTTON_HEIGHT / 2 - BEGIN_SCANNING_TEXT_SIZE / 2,
              "Confirm", BEGIN_SCANNING_TEXT_SIZE);
    confirm_label.set_alignment(LABEL_ALIGN_CENTER);

    scene.add_object(&cancel_button);
    scene.add_object(&cancel_label);
    scene.add_object(&confirm_button);
    scene.add_object(&confirm_label);

    lcd_set_background(BEGIN_SCANNING);
    gui_set_current_scene(&scene);

    return STATUS_OK;
}

/**********************/
/* INSERT CARD SCREEN */
/**********************/

static Scene s_insert_card_scene;

static Status load_insert_card_scene() {
    lcd_set_background(INSERT_CARD);
    gui_set_current_scene(&s_insert_card_scene);

    return STATUS_OK;
}

static Status update_insert_card_scene() {
    // If the card is inserted, go to the file list scene
    if (filesystem_is_card_inserted() && filesystem_is_mounted()) {
        load_file_list_scene();
        update_file_list(true);
    }

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

    // Sending file screen
    if (gui_get_current_scene() == &sending_file_scene_ctx.scene) {
        if (get_tick_ms() - sending_file_scene_ctx.last_update_tick >= 200) {
            update_sending_file_scene();
            sending_file_scene_ctx.last_update_tick = get_tick_ms();
        }
    }

    // Insert card screen
    if (gui_get_current_scene() == &s_insert_card_scene) {
        update_insert_card_scene();
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