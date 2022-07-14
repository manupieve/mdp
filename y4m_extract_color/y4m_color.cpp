#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "mat.h"
#include "types.h"

double upsample(const mat<uint8_t>& img, int r, int c) {
	return img(r/2, c/2);
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
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

	mat<vec3b> frame(rows, cols);

	mat<uint8_t> Y(rows, cols);
	mat<uint8_t> Cb(rows/2, cols/2);
	mat<uint8_t> Cr(rows/2, cols/2);

	while (is.peek() != EOF) {
		is >> tagged_field;
		if (tagged_field != "FRAME")
			return false;

		is.get();

		is.read(Y.rawdata(), Y.rawsize());
		is.read(Cb.rawdata(), Cb.rawsize());
		is.read(Cr.rawdata(), Cr.rawsize());

		for (int r = 0; r < frame.rows(); ++r) {
			for (int c = 0; c < frame.cols(); ++c) {
				double y = Y(r, c);
				double cb = upsample(Cb, r, c);
				double cr = upsample(Cr, r, c);

				y = y < 16 ? 16 : y > 235 ? 235 : y;
				cb = cb < 16 ? 16 : cb > 240 ? 240 : cb;
				cr = cr < 16 ? 16 : cr > 240 ? 240 : cr;

				y -= 16;
				cb -= 128;
				cr -= 128;

				double dr = 1.164*y + 0.000*cb + 1.596*cr;
				double dg = 1.164*y - 0.392*cb - 0.813*cr;
				double db = 1.164*y + 2.017*cb + 0.000*cr;

				dr = dr < 0 ? 0 : dr > 255 ? 255 : dr;
				dg = dg < 0 ? 0 : dg > 255 ? 255 : dg;
				db = db < 0 ? 0 : db > 255 ? 255 : db;

				frame(r, c) = { uint8_t(dr), uint8_t(dg), uint8_t(db) };
			}
		}

		frames.push_back(frame);
	}

	return true;
}