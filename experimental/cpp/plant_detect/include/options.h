#pragma once

namespace Options {
void setCamera(int id);
int getCamera();

void setResolution(int width, int height);
void getResolution(int& width, int& height);
}  // namespace Options