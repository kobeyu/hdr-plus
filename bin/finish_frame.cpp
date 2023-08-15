#include <Halide.h>
#include <align_and_merge.h>
#include <src/Burst.h>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " dir_path out_img raw_img"
                  << std::endl;
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

    std::cerr << "raw size(w/h): " << burst.GetWidth() << ", "
              << burst.GetHeight() << std::endl;
    return 0;
}
