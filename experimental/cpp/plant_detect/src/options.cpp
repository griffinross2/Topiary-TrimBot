#include "options.h"

int s_camera_id = 0;

void Options::setCamera(int id) {
    s_camera_id = id;
}

int Options::getCamera() {
    return s_camera_id;
}