#include "options.h"

int s_cameraId = 0;

void Options::setCamera(int id) {
    s_cameraId = id;
}

int Options::getCamera() {
    return s_cameraId;
}