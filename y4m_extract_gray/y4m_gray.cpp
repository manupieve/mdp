#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "mat.h"

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	std::ifstream is(filename, std::ios::binary);
	if (!is)
		return false;

	std::string stream_header;
	is >> stream_header;
	if (stream_header != "YUV4MPEG2")
		return false;

	std::string tagged_field;
	int rows = 0, cols = 0;
	while (is.peek() != '\n') {
		is >> tagged_field;
		if (tagged_field[0] == 'H') {
			rows = std::stoi(&tagged_field[1]);
		}
		if (tagged_field[0] == 'W') {
			cols = std::stoi(&tagged_field[1]);
		}
		if (tagged_field[0] == 'C') {
			if (tagged_field != "C420jpeg")
				return false;
		}
	}

	is.get();

	if (rows == 0 || cols == 0)
		return false;

	mat<uint8_t> frame(rows, cols);

	while (is.peek() != EOF) {
		is >> tagged_field;
		if (tagged_field != "FRAME")
			return false;

		is.get();

		is.read(frame.rawdata(), frame.rawsize());

		for (int i = 0; i < (frame.rawsize() / 4 * 2); ++i)
			is.get();

		frames.push_back(frame);
	}

	return true;
}