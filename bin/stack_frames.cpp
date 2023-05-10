#include <Halide.h>
#include <align_and_merge.h>
#include <jsoncpp/json/json.h>
#include <src/Burst.h>
#include <stdio.h>

#include <fstream>
#include <iostream>

Halide::Runtime::Buffer<uint16_t> align_and_merge(
    Halide::Runtime::Buffer<uint16_t> burst) {
    if (burst.channels() < 2) {
        return {};
    }
    Halide::Runtime::Buffer<uint16_t> merged_buffer(burst.width(),
                                                    burst.height());
    align_and_merge(burst, merged_buffer);
    return merged_buffer;
}

void dump_params(Burst& burst, const std::string& dir_path) {
    // dump parameters to json file
    Json::Value root;
    root["img_width"] = burst.ToBuffer().width();
    root["img_height"] = burst.ToBuffer().height();
    root["black_point"] = burst.GetBlackLevel();
    root["white_point"] = burst.GetWhiteLevel();
    const WhiteBalance wb = burst.GetWhiteBalance();
    Json::Value rggb;
    rggb.append(wb.r);
    rggb.append(wb.g0);
    rggb.append(wb.g1);
    rggb.append(wb.b);

    root["RGGB"] = rggb;

    auto ccm = burst.GetColorCorrectionMatrix();

    for (int i = 0; i < ccm.height(); i++) {
        Json::Value row;
        for (int j = 0; j < ccm.width(); j++) {
            row.append(ccm(i, j));
        }
        root["ccm"].append(row);
    }

    // Open a file stream for writing
    std::ofstream file(dir_path + "/params.json");

    //  Write the JSON data to the file
    Json::StyledWriter writer;
    file << writer.write(root);

    // Close the file stream
    file.close();
}

void dump_buffer(const Halide::Runtime::Buffer<uint16_t>& merged_buf,
                 const std::string& dir_path) {
    int size = merged_buf.width() * merged_buf.height() * sizeof(uint16_t);

    std::ofstream outfile(dir_path + "/merged.bin", std::ios::binary);
    outfile.write(reinterpret_cast<char*>(merged_buf.data()), size);
    outfile.close();
}

void dump(Burst& burst, const Halide::Runtime::Buffer<uint16_t>& merged_buf,
          const std::string& dir_path) {
    dump_params(burst, dir_path);
    dump_buffer(merged_buf, dir_path);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " dir_path out_img raw_img1 raw_img2 [...]" << std::endl;
        return 1;
    }

    int i = 1;
    if (argc - i < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " dir_path out_img raw_img1 raw_img2 [...]" << std::endl;
        return 1;
    }

    const std::string dir_path = argv[i++];
    const std::string out_name = argv[i++];

    std::vector<std::string> in_names;
    while (i < argc) in_names.push_back(argv[i++]);

    Burst burst(dir_path, in_names);

    std::cerr << "raw size(w/h): " << burst.GetWidth() << ", " << burst.GetHeight() << std::endl;

    const auto merged = align_and_merge(burst.ToBuffer());
    std::cerr << "merged size(w/h): " << merged.width() << ", " << merged.height()
              << std::endl;

    dump(burst, merged, dir_path);

    // const RawImage& raw = burst.GetRaw(0);
    // const std::string merged_filename = dir_path + "/" + out_name;
    // raw.WriteDng(merged_filename, merged);

    return EXIT_SUCCESS;
}
