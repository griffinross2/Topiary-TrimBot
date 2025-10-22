#include "application.h"

#include "options_layer.h"
#include "camera_block.h"
#include "int_input_block.h"
#include "display_block.h"
#include "grayscale_block.h"
#include "debug_layer.h"
#include "block.h"

int main(int argc, char** argv) {
    Application app;

    int ret = 0;

    if ((ret = app.init()) != 0) {
        return ret;
    }

    // Push layers
    // app.pushLayer<CameraLayer>();
    // app.pushLayer<OptionsLayer>();
    app.pushLayer<DebugLayer>();
    createBlock<IntInputBlock>();
    createBlock<CameraBlock>();
    createBlock<DisplayBlock>();
    createBlock<GrayscaleBlock>();

    ret = app.run();
    app.shutdown();

    return ret;
}
