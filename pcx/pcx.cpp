#include "pcx.h"

bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is)
		return false;

	uint8_t manufacturer;
	is.read(reinterpret_cast<char*>(&manufacturer), sizeof(uint8_t));
	if (manufacturer != 0x0A)
		return false;

	uint8_t version;
	is.read(reinterpret_cast<char*>(&version), sizeof(uint8_t));
	if (version != 0x05)
		return false;

	uint8_t encoding;
	is.read(reinterpret_cast<char*>(&encoding), sizeof(uint8_t));
	if (encoding != 0x01)
		return false;

	uint8_t bits_per_plane;
	is.read(reinterpret_cast<char*>(&bits_per_plane), sizeof(uint8_t));

	uint16_t x_min, y_min, x_max, y_max;
	is.read(reinterpret_cast<char*>(&x_min), sizeof(uint16_t));
	is.read(reinterpret_cast<char*>(&y_min), sizeof(uint16_t));
	is.read(reinterpret_cast<char*>(&x_max), sizeof(uint16_t));
	is.read(reinterpret_cast<char*>(&y_max), sizeof(uint16_t));

	for (size_t i = 0; i < 53; ++i)
		is.get();

	uint8_t color_planes;
	is.read(reinterpret_cast<char*>(&color_planes), sizeof(uint8_t));

	uint16_t bytes_per_plane_line;
	is.read(reinterpret_cast<char*>(&bytes_per_plane_line), sizeof(uint16_t));

	for (size_t i = 0; i < 60; ++i)
		is.get();

	int x_size, y_size;
	x_size = x_max - x_min + 1;
	y_size = y_max - y_min + 1;

	img.resize(y_size, x_size);

	int total_bytes;
	total_bytes = color_planes * bytes_per_plane_line;

	is.seekg(-769, std::ios::end);
	if (is.get() != 0x0C)
		return false;

	uint8_t red, green, blue;
	std::map<size_t, vec3b> palette;

	for (size_t i = 0; i < 256; ++i) {
		is.read(reinterpret_cast<char*>(&red), sizeof(uint8_t));
		is.read(reinterpret_cast<char*>(&green), sizeof(uint8_t));
		is.read(reinterpret_cast<char*>(&blue), sizeof(uint8_t));
		palette[i] = { red, green, blue };
	}

	is.seekg(128, std::ios::beg);

	bool full_line = false;
	uint8_t byte, count, data;
	std::vector<uint8_t> line;
	int subtotal = 0;
	auto it = img.begin();

	for (int y = 0; y < y_size; ++y) {
		while (true) {
			if (full_line)
				break;
			is.read(reinterpret_cast<char*>(&byte), sizeof(uint8_t));
			if (((byte & 0b11000000) >> 6) == 0b00000011) {
				count = byte & 0b00111111;
				data = is.get();
			}
			else {
				count = 1;
				data = byte;
			}
			for (int i = 0; i < count; ++i) {
				line.push_back(data);
				++subtotal;
				if (total_bytes == subtotal) {
					full_line = true;
					break;
				}
			}
		}
		full_line = false;
		subtotal = 0;
		for (const auto& x : line) {
			if (x_size == subtotal)
				break;
			*it = palette[x];
			++subtotal;
			++it;
		}
		line.clear();
		subtotal = 0;
	}
	
	return true;
}