#include <Halide.h>
#include <src/Burst.h>

#include <iostream>

#include "finish_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <include/stb_image_write.h>

static bool save_png(const std::string& dir_path, const std::string& img_name,
                     const Halide::Runtime::Buffer<uint8_t>& img) {
    const std::string img_path = dir_path + "/" + img_name;

    const int stride_in_bytes = img.width() * img.channels();
    if (!stbi_write_png(img_path.c_str(), img.width(), img.height(),
                        img.channels(), img.data(), stride_in_bytes)) {
        std::cerr << "Unable to write output image '" << img_name << "'"
                  << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " dir_path out_img raw_img"
                  << std::endl;
        return 1;
    }

    int i = 1;

    const std::string dir_path = argv[i++];
    const std::string out_name = argv[i++];

    std::vector<std::string> in_names;
    while (i < argc) in_names.push_back(argv[i++]);

    Burst burst(dir_path, in_names);

    std::cerr << "Black point: " << burst.GetBlackLevel() << std::endl;
    std::cerr << "White point: " << burst.GetWhiteLevel() << std::endl;
    std::cerr << "raw size(w/h): " << burst.GetWidth() << ", "
              << burst.GetHeight() << std::endl;

    const WhiteBalance wb = burst.GetWhiteBalance();
    std::cerr << "RGGB: " << wb.r << " " << wb.g0 << " " << wb.g1 << " " << wb.b
              << std::endl;

    Halide::Runtime::Buffer<uint16_t> imgs = burst.ToBuffer(false);
    // if (imgs.dimensions() != 3 || imgs.extent(2) < 2) {
    //     throw std::invalid_argument(
    //         "The input of HDRPlus must be a 3-dimensional buffer with at "
    //         "least two channels.");
    // }

    const int width = burst.GetWidth();
    const int height = burst.GetHeight();
    Halide::Runtime::Buffer<uint8_t> output_img(3, width, height);

    const int cfa_pattern = static_cast<int>(burst.GetCfaPattern());
    Compression c = 3.8f;
    Gain g = 1.1f;

    auto ccm = burst.GetColorCorrectionMatrix();

    finish_image(imgs, burst.GetBlackLevel(), burst.GetWhiteLevel(), wb.r,
                 wb.g0, wb.g1, wb.b, cfa_pattern, ccm, c, g, output_img);

    output_img.transpose(0, 1);
    output_img.transpose(1, 2);
    if (!save_png(dir_path, out_name, output_img)) {
        return 0;
    }
}
