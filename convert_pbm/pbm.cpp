#include "pbm.h"

bool BinaryImage::ReadFromPBM(const std::string& filename) {
	std::ifstream is(filename, std::ios::binary);
	if (!is)
		return false;

	std::string magic_number;
	std::getline(is, magic_number);
	if (magic_number != "P4")
		return false;

	std::string comment;
	if (is.peek() == '#')
		std::getline(is, comment);

	uint32_t W, H;
	is >> W >> H;
	if (W < 1 || H < 1 || is.get() != '\n')
		return false;

	this->W = W;
	this->H = H;

	uint8_t byte;
	while (is.read(reinterpret_cast<char*>(&byte), 1))
		this->ImageData.push_back(byte);

	return true;
}

Image BinaryImageToImage(const BinaryImage& bimg) {
	Image img;
	img.W = bimg.W;
	img.H = bimg.H;

	int count = 0;
	size_t pad;
	uint8_t byte;

	for (auto it = bimg.ImageData.begin(); it != bimg.ImageData.end(); ++it) {
		if (((count + 1) % (img.W / 8 + 1)) == 0)
			pad = 8 - (img.W % 8);
		else
			pad = 0;
		for (size_t i = 0; i < (8-pad); ++i) {
			if (((*it << i) >> 7) & 1)
				byte = 0;
			else
				byte = 255;
			img.ImageData.push_back(byte);
		}
		++count;
	}

	return img;
}