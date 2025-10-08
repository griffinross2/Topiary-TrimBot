#include "application.h"

#include "camera_layer.h"
#include "options_layer.h"

int main(int argc, char** argv) {
    Application app;

    int ret = 0;

    if ((ret = app.init()) != 0) {
        return ret;
    }

    // Push layers
    app.pushLayer<OptionsLayer>();
    app.pushLayer<CameraLayer>();

    ret = app.run();
    app.shutdown();

    return ret;
}
