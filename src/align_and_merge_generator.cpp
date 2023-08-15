#include <Halide.h>

#include "align.h"
#include "finish.h"
#include "merge.h"

namespace {

class StackFrames : public Halide::Generator<StackFrames> {
   public:
    // 'inputs' is really a series of raw 2d frames; extent[2] specifies the
    // count
    Input<Halide::Buffer<uint16_t>> inputs{"inputs", 3};
    // Merged buffer
    Output<Halide::Buffer<uint16_t>> output{"output", 2};

    void generate() {
        Func alignment = align(inputs, inputs.width(), inputs.height());
        Func merged = merge(inputs, inputs.width(), inputs.height(),
                            inputs.dim(2).extent(), alignment);
        output = merged;
    }

    void schedule() {
        if (using_autoscheduler()) {
            inputs.set_estimates({{0, 4048}, {0, 3036}, {0, 6}});
            output.set_estimates({{0, 4048}, {0, 3036}});
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(StackFrames, align_and_merge)
