#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

template<typename T>
struct mat {
    size_t rows_, cols_;
    std::vector<T> data_;

    mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows*cols) {}

    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }

    size_t size() const { return rows_ * cols_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    const char* rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t rawsize() const { return size() * sizeof(T); }
};

int main(int argc, char *argv[]) {
    if (argc != 3)
        return EXIT_FAILURE;

    std::ifstream is(argv[1], std::ios::binary);
    if (!is)
        return EXIT_FAILURE;

    char magic[4];
    is.read(magic, 4);
    if (magic[0] != 'q' || magic[1] != 'o' || magic[2] != 'i' || magic[3] != 'f')
        return EXIT_FAILURE;

    uint32_t width = 0;
    for (size_t i = 0; i < 4; ++i)
        width = (width << 8) | (is.get());
    uint32_t height = 0;
    for (size_t i = 0; i < 4; ++i)
        height = (height << 8) | (is.get());
    if (width < 1 || height < 1)
        return EXIT_FAILURE;

    uint8_t channels, colorspace;
    channels = is.get();
    colorspace = is.get();
    if (channels != 3 && channels != 4)
        return EXIT_FAILURE;
    if (colorspace != 0 && colorspace != 1)
        return EXIT_FAILURE;

    using rgba = std::array<uint8_t, 4>;
    mat<rgba> img(height, width);

    rgba previous_pixel = { 0, 0, 0, 255 };
    std::array<rgba, 64> previous_pixels({ 0, 0, 0, 0 });
    for (size_t i = 0; i < img.size();) {
        uint8_t tag;
        tag = is.get();
        if (tag == 0b11111110) {
            uint8_t r, g, b, a;
            r = is.get();
            g = is.get();
            b = is.get();
            a = previous_pixel[3];
            img[i] = { r, g, b, a };
            previous_pixel = { r, g, b, a };
            size_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
            previous_pixels[index_position] = { r, g, b, a };
            ++i;
            continue;
        }
        if (tag == 0b11111111) {
            uint8_t r, g, b, a;
            r = is.get();
            g = is.get();
            b = is.get();
            a = is.get();
            img[i] = { r, g, b, a };
            previous_pixel = { r, g, b, a };
            size_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
            previous_pixels[index_position] = { r, g, b, a };
            ++i;
            continue;
        }
        if ((tag >> 6) == 0b00000000) {
            uint8_t r, g, b, a;
            r = previous_pixels[tag][0];
            g = previous_pixels[tag][1];
            b = previous_pixels[tag][2];
            a = previous_pixels[tag][3];
            img[i] = { r, g, b, a };
            previous_pixel = { r, g, b, a };
            size_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
            previous_pixels[index_position] = { r, g, b, a };
            ++i;
            continue;
        }
        if ((tag >> 6) == 0b00000001) {
            uint8_t dr, dg, db;
            dr = ((tag & 0b00110000) >> 4) - 2;
            dg = ((tag & 0b00001100) >> 2) - 2;
            db = ((tag & 0b00000011) >> 0) - 2;
            uint8_t r, g, b, a;
            r = previous_pixel[0] + dr;
            g = previous_pixel[1] + dg;
            b = previous_pixel[2] + db;
            a = previous_pixel[3];
            img[i] = { r, g, b, a };
            previous_pixel = { r, g, b, a };
            size_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
            previous_pixels[index_position] = { r, g, b, a };
            ++i;
            continue;
        }
        if ((tag >> 6) == 0b00000010) {
            uint8_t dg;
            dg = (tag & 0b00111111) - 32;
            uint8_t byte;
            byte = is.get();
            uint8_t dr_dg, db_dg;
            dr_dg = ((byte & 0b11110000) >> 4) - 8;
            db_dg = ((byte & 0b00001111) >> 0) - 8;
            uint8_t r, g, b, a;
            r = previous_pixel[0] + dg + dr_dg;
            g = previous_pixel[1] + dg;
            b = previous_pixel[2] + dg + db_dg;
            a = previous_pixel[3];
            img[i] = { r, g, b, a };
            previous_pixel = { r, g, b, a };
            size_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
            previous_pixels[index_position] = { r, g, b, a };
            ++i;
            continue;
        }
        if ((tag >> 6) == 0b00000011) {
            uint8_t run;
            run = (tag & 0b00111111) + 1;
            uint8_t r, g, b, a;
            r = previous_pixel[0];
            g = previous_pixel[1];
            b = previous_pixel[2];
            a = previous_pixel[3];
            for (size_t j = 0; j < run; ++j) {
                img[i] = { r, g, b, a };
                previous_pixel = { r, g, b, a };
                size_t index_position = (r * 3 + g * 5 + b * 7 + a * 11) % 64;
                previous_pixels[index_position] = { r, g, b, a };
                ++i;
            }
            continue;
        }
    }

    std::ofstream os(argv[2], std::ios::binary);
    os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.rawsize());

    return EXIT_SUCCESS;
}