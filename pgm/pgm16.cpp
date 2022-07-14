#include "pgm16.h"

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {
	std::ifstream is(filename, std::ios::binary);
	if (!is)
		return false;

	std::string magic_number;
	std::getline(is, magic_number);
	if (magic_number != "P5")
		return false;

	std::string comment;
	if (is.peek() == '#')
		std::getline(is, comment);

	uint32_t W, H;
	is >> W >> H >> maxvalue;

	if (W < 1 || H < 1 || is.get() != '\n')
		return false;

	img.resize(H, W);

	uint8_t first_byte, second_byte;
	uint16_t value;
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			is.read(reinterpret_cast<char*>(&first_byte), 1);
			if (maxvalue < 256) {
				value = first_byte;
			}
			else {
				is.read(reinterpret_cast<char*>(&second_byte), 1);
				value = first_byte << 8 | second_byte;
			}
			img(r, c) = value;
		}
	}

	return true;
}