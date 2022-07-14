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