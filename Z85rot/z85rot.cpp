#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

void encode(const int N, const std::string input_file, const std::string output_file) {
	std::string alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";
	
	std::ifstream is(input_file, std::ios::binary);
	if (!is)
		exit(EXIT_FAILURE);

	std::ofstream os(output_file, std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	char magic_number[2];
	is.read(magic_number, 2);
	if (magic_number[0] != 'P' || magic_number[1] != '6')
		exit(EXIT_FAILURE);
	is.get();

	std::string comment;
	if (is.peek() == '#')
		std::getline(is, comment);

	int width, height, maxval;
	is >> width >> height >> maxval;
	if (width < 1 || height < 1 || maxval < 1 || maxval > 65535)
		exit(EXIT_FAILURE);
	is.get();

	std::vector<uint8_t> data;
	for (int i = 0; i < width * height * 3; ++i)
		data.push_back(is.get());
	if (is.peek() != EOF)
		exit(EXIT_FAILURE);
	while ((data.size() % 4) != 0)
		data.push_back(0);

	os << width << ',' << height << ',';
	
	int count = 0;
	uint32_t value = 0;
	std::vector<int> indexes;
	for (const auto& octet : data) {
		++count;
		value = (value << 8) | (octet);
		if (count == 4) {
			for (int i = 0; i < 5; ++i) {
				indexes.insert(indexes.begin(), value % 85);
				value /= 85;
			}
			for (const auto& index : indexes) {
				os << alphabet[index];
				std::rotate(alphabet.begin(), alphabet.end() - N, alphabet.end());
			}
			count = 0;
			value = 0;
			indexes.clear();
		}
	}
}

void decode(const int N, const std::string input_file, const std::string output_file) {
	std::string alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-:+=^!/*?&<>()[]{}@%$#";
	
	std::ifstream is(input_file, std::ios::binary);
	if (!is)
		exit(EXIT_FAILURE);

	std::ofstream os(output_file, std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	int width, height;
	char first_comma, second_comma;
	is >> width >> first_comma >> height >> second_comma;
	if (width < 1 || height < 1 || first_comma != ',' || second_comma != ',')
		exit(EXIT_FAILURE);

	os << "P6" << '\n' << width << ' ' << height << '\n' << "255" << '\n';

	char character;
	int count = 0;
	std::vector<char> characters;
	uint32_t value = 0;
	int written = 0;
	uint8_t byte;
	while (is.read(&character, 1)) {
		++count;
		characters.push_back(character);
		if (count == 5) {
			for (int i = 0; i < 5; ++i) {
				int index = int(alphabet.find(characters[i]));
				value += index * uint32_t(std::pow(85, 4 - i));
				std::rotate(alphabet.begin(), alphabet.end() - N, alphabet.end());
			}
			if (written < width * height * 3) {
				++written;
				byte = (value & 0xFF000000) >> 24;
				os.write(reinterpret_cast<char*>(&byte), 1);
			}
			if (written < width * height * 3) {
				++written;
				byte = (value & 0x00FF0000) >> 16;
				os.write(reinterpret_cast<char*>(&byte), 1);
			}
			if (written < width * height * 3) {
				++written;
				byte = (value & 0x0000FF00) >> 8;
				os.write(reinterpret_cast<char*>(&byte), 1);
			}
			if (written < width * height * 3) {
				++written;
				byte = (value & 0x000000FF) >> 0;
				os.write(reinterpret_cast<char*>(&byte), 1);
			}
			count = 0;
			characters.clear();
			value = 0;
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 5)
		return EXIT_FAILURE;

	std::string command(argv[1]);
	if (command == "c")
		encode(std::stoi(argv[2]), argv[3], argv[4]);
	else if (command == "d")
		decode(std::stoi(argv[2]), argv[3], argv[4]);
	else
		return EXIT_FAILURE;
	
	return EXIT_SUCCESS;
}