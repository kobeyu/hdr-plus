#include <Halide.h>

#include "finish.h"

namespace {

class FinishFrame : public Halide::Generator<FinishFrame> {
   public:
    Input<Halide::Buffer<uint16_t>> input{"input", 2};
    Input<uint16_t> black_point{"black_point"};
    Input<uint16_t> white_point{"white_point"};
    Input<float> white_balance_r{"white_balance_r"};
    Input<float> white_balance_g0{"white_balance_g0"};
    Input<float> white_balance_g1{"white_balance_g1"};
    Input<float> white_balance_b{"white_balance_b"};
    Input<int> cfa_pattern{"cfa_pattern"};
    Input<Halide::Buffer<float>> ccm{"ccm",
                                     2};  // ccm - color correction matrix
    Input<float> compression{"compression"};
    Input<float> gain{"gain"};

    Output<Halide::Buffer<uint8_t>> output{"output", 3};

    void generate() {
        CompiletimeWhiteBalance wb{white_balance_r, white_balance_g0,
                                   white_balance_g1, white_balance_b};
        Func finished =
            finish(input, input.width(), input.height(), black_point,
                   white_point, wb, cfa_pattern, ccm, compression, gain);
        output = finished;
    }
    void schedule() {
        if (using_autoscheduler()) {
            input.set_estimates({{0, 4048}, {0, 3036}});
            output.set_estimates({{0, 4048}, {0, 3036}, {0, 3}});
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(FinishFrame, finish_image)
