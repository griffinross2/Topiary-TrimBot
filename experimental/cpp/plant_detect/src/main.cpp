#include "application.h"

#include "options_layer.h"
#include "camera_block.h"
#include "int_input_block.h"
#include "display_block.h"
#include "grayscale_block.h"
#include "mean_shift_detect_block.h"
#include "rescale_block.h"
#include "still_frame_block.h"
#include "depth_block.h"
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
    createBlock<IntInputBlock>();
    createBlock<CameraBlock>();
    createBlock<DisplayBlock>();
    createBlock<DisplayBlock>();
    createBlock<DisplayBlock>();
    createBlock<DisplayBlock>();
    // createBlock<GrayscaleBlock>();
    createBlock<RescaleBlock>();
    // createBlock<RescaleBlock>();
    // createBlock<MeanShiftDetectBlock>();
    createBlock<StillFrameBlock>();
    createBlock<StillFrameBlock>();
    createBlock<DepthBlock>();

    ret = app.run();
    app.shutdown();

    return ret;
}
