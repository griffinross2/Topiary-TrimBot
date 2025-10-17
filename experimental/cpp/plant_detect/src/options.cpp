#include "options.h"

int s_cameraId = 0;
int s_cameraWidth = 0;
int s_cameraHeight = 0;

void Options::setCamera(int id) {
    s_cameraId = id;
}

int Options::getCamera() {
    return s_cameraId;
}

void Options::setResolution(int width, int height) {
    s_cameraWidth = width;
    s_cameraHeight = height;
}

void Options::getResolution(int& width, int& height) {
    width = s_cameraWidth;
    height = s_cameraHeight;
}