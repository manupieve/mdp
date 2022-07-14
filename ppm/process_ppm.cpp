#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "mat.h"
#include "ppm.h"

bool LoadPPM(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is)
		return false;

	std::string magic_number;
	std::getline(is, magic_number);
	if (magic_number != "P6")
		return false;

	std::string comment;
	if (is.peek() == '#')
		std::getline(is, comment);

	int width, height, maxval;
	is >> width >> height >> maxval;
	if (width <= 0 || height <= 0 || maxval <= 0 || maxval >= 65536)
		return false;

	img.resize(height, width);

	is.get();

	uint8_t red, green, blue;
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			red = is.get();
			green = is.get();
			blue = is.get();
			img(r, c) = { red, green, blue };
		}
	}

	return true;
}

void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
	img_r.resize(img.rows(), img.cols());
	img_g.resize(img.rows(), img.cols());
	img_b.resize(img.rows(), img.cols());

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img_r(r, c) = img(r, c)[0];
			img_g(r, c) = img(r, c)[1];
			img_b(r, c) = img(r, c)[2];
		}
	}
}