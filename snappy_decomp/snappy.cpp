#include <string>
#include <iostream>
#include <fstream>
#include <vector>

void syntax(const std::string& msg) {
	std::cout << "Syntax: " << msg << '\n';
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "Error: " << msg << '\n';
	exit(EXIT_FAILURE);
}

size_t read_preamble(std::ifstream& is) {
	uint8_t byte;
	std::vector<uint8_t> values;
	size_t original_size = 0;
	
	while (true) {
		is.read(reinterpret_cast<char*>(&byte), 1);
		uint8_t val = byte & 0b01111111;
		values.push_back(val);
		uint8_t msb = byte & 0b10000000;
		if (msb == 0)
			break;
	}

	for (int64_t i = values.size() - 1; i >= 0; --i) {
		original_size = original_size << 7;
		original_size += values[i];
	}

	return original_size;
}

void snappy_decomp(const std::string& input_filename, const std::string& output_filename) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		error("cannot open file " + output_filename);

	size_t original_size = read_preamble(is);

	uint8_t tag;
	std::vector<uint8_t> data;

	while (is.read(reinterpret_cast<char*>(&tag), 1)) {
		uint8_t type = tag & 0b00000011;
		if (type == 0b00) {
			/* Literal */
			size_t length = (tag & 0b11111100) >> 2;
			
			if (length <= 59) {
				++length;
			}
			else {
				size_t count = length - 59;
				length = 0;
				is.read(reinterpret_cast<char*>(&length), count);
				++length;
			}
			
			uint8_t byte;
			for (size_t i = 0; i < length; ++i) {
				is.read(reinterpret_cast<char*>(&byte), 1);
				os.write(reinterpret_cast<const char*>(&byte), 1);
				data.push_back(byte);
			}
		}
		else {
			/* Copy */
			size_t length = 0;
			size_t offset = 0;
			
			if (type == 0b01) {
				length = (tag & 0b00011100) >> 2;
				length += 4;
				offset = (tag & 0b11100000);
				offset = offset << 3;
				uint8_t byte;
				is.read(reinterpret_cast<char*>(&byte), 1);
				offset += byte;
			}
			else if (type == 0b10) {
				length = (tag & 0b11111100) >> 2;
				++length;
				uint16_t offset16;
				is.read(reinterpret_cast<char*>(&offset16), 2);
				offset = offset16;
			}
			else if (type == 0b11) {
				length = (tag & 0b11111100) >> 2;
				++length;
				uint32_t offset32;
				is.read(reinterpret_cast<char*>(&offset32), 4);
				offset = offset32;
			}

			size_t size = data.size();
			for (size_t i = 0; i < length; ++i) {
				uint8_t byte = data.at(size - offset + i);
				os.write(reinterpret_cast<const char*>(&byte), 1);
				data.push_back(byte);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 3)
		syntax("snappy_decomp <input file> <output file>");

	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	snappy_decomp(input_filename, output_filename);
	
	return EXIT_SUCCESS;
}