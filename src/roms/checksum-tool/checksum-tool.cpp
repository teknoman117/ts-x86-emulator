/* generate the checksum of the option rom */

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

template <typename T>
T next_power_of_2(T v) {
    T value = 1;
    while (value < v)
        value <<= 1;
    return value;
}

int main(int argc, char** argv) {
    // ensure we have enough arguments
    if (argc < 3) {
        std::cerr << "Too few arguments" << std::endl;
        if (argc >= 1) {
            std::cerr << "Usage: " << argv[0] << " <input file> <output file>" << std::endl;
        }
        return EXIT_FAILURE;
    }

    // read the contents of the rom file
    std::filesystem::path input_path(argv[1]);
    std::ifstream input_file(input_path, std::ios::binary);
    input_file.seekg(0, std::ios::end);
    auto length = input_file.tellg();
    input_file.seekg(0, std::ios::beg);

    auto output_length = next_power_of_2<size_t>(length);
    std::vector<uint8_t> contents(output_length, 0);
    input_file.read((char *) contents.data(), length);
    input_file.close();

    // compute the checksum and write it back to the file
    auto checksum = std::accumulate(contents.begin(), contents.end(), 0) % 256;
    checksum = checksum ? 256 - checksum : 0;
    contents[2] = output_length / 512;
    contents.back() = checksum;

    // write the contents of the rom back to file
    std::filesystem::path output_path(argv[2]);
    std::ofstream output_file(output_path, std::ios::binary);
    output_file.write((char *) contents.data(), contents.size());
    output_file.close();
    return EXIT_SUCCESS;
}