#include <string>
#include <iostream>
#include <fstream>
#include <vector>

void read_certificate(const std::string& input_filename) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		exit(EXIT_FAILURE);

	std::ofstream os("encoded.txt", std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	std::string begin_certificate, end_certificate;
	char c;

	std::getline(is, begin_certificate);
	if (begin_certificate.find("BEGIN CERTIFICATE") == std::string::npos)
		exit(EXIT_FAILURE);

	while (is.peek() != '-') {
		is.read(&c, 1);
		os.write(&c, 1);
	}

	std::getline(is, end_certificate);
	if (end_certificate.find("END CERTIFICATE") == std::string::npos)
		exit(EXIT_FAILURE);
}

void decode_base64() {
	std::ifstream is("encoded.txt", std::ios::binary);
	if (!is)
		exit(EXIT_FAILURE);

	std::ofstream os("decoded.bin", std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	char c;
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	uint32_t buffer = 0, count = 0;
	uint8_t first_byte, second_byte, third_byte;
	
	while (is.read(&c, 1)) {
		if (c != '=') {
			int val = int(alphabet.find(c));
			if (val != -1) {
				buffer <<= 6;
				buffer |= val;
				count = count + 1;
				if (count == 4) {
					first_byte = uint8_t(buffer >> 16);
					second_byte = uint8_t(buffer >> 8);
					third_byte = uint8_t(buffer >> 0);
					os.write(reinterpret_cast<char*>(&first_byte), 1);
					os.write(reinterpret_cast<char*>(&second_byte), 1);
					os.write(reinterpret_cast<char*>(&third_byte), 1);
					buffer = 0;
					count = 0;
				}
			}
		}
	}

	if (count == 2) {
		first_byte = uint8_t(buffer >> 4);
		os.write(reinterpret_cast<char*>(&first_byte), 1);
	}

	if (count == 3) {
		first_byte = uint8_t(buffer >> 10);
		os.write(reinterpret_cast<char*>(&first_byte), 1);
		second_byte = uint8_t(buffer >> 2);
		os.write(reinterpret_cast<char*>(&second_byte), 1);
	}
}

void decode_bin() {
	std::ifstream is("decoded.bin", std::ios::binary);
	if (!is)
		exit(EXIT_FAILURE);

	std::ofstream os("output.txt", std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	uint8_t identifier_byte, class_bits, pc_bit, byte, go, num_bytes;
	uint64_t tag, length, header_length;
	std::vector<uint8_t> content;

	while (is.read(reinterpret_cast<char*>(&identifier_byte), 1)) {
		header_length = 1;
		class_bits = ((identifier_byte & 0b11000000) >> 6);
		pc_bit = ((identifier_byte & 0b00100000) >> 5);
		tag = 0;
		tag = ((identifier_byte & 0b00011111) >> 0);
		if (tag == 31) {
			tag = 0;
			do {
				is.read(reinterpret_cast<char*>(&byte), 1);
				++header_length;
				go = ((byte & 0b10000000) >> 7);
				tag <<= 7;
				tag |= (byte & 0b01111111);
			} while (go);
		}
		is.read(reinterpret_cast<char*>(&byte), 1);
		++header_length;
		length = 0;
		if (((byte & 0b10000000) >> 7) == 0) {
			length = (byte & 0b01111111);
		}
		else if (((byte & 0b10000000) >> 7) == 1) {
			num_bytes = (byte & 0b01111111);
			while (num_bytes-- > 0) {
				is.read(reinterpret_cast<char*>(&byte), 1);
				++header_length;
				length <<= 8;
				length |= byte;
			}
		}
		os << std::to_string(class_bits) << ", " << std::to_string(pc_bit) << ", " << std::to_string(tag) << ", " << std::to_string(header_length) << "+" << std::to_string(length);
		if (class_bits == 0 && pc_bit == 0) {
			content.resize(length);
			is.read(reinterpret_cast<char*>(content.data()), length);
		}
		if (tag == 19) {
			os << ": ";
			os.write(reinterpret_cast<char*>(content.data()), length);
		}
		os << "\n";
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2)
		return EXIT_FAILURE;

	read_certificate(argv[1]);

	decode_base64();

	decode_bin();

	return EXIT_SUCCESS;
}