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

class packbits {
	std::ifstream& is;
	std::ofstream& os;
	std::vector<uint8_t> buffer;
	uint8_t byte = 0;
	int8_t command = 0;

public:
	packbits(std::ifstream& is, std::ofstream& os) : is(is), os(os) {}

	void compress() {
		while (is.read(reinterpret_cast<char*>(&byte), 1)) {
			if (buffer.size() == 0) {
				buffer.push_back(byte);
			}
			else if (buffer.back() == byte && command <= 0) {
				if (command == -127) {
					os.write(reinterpret_cast<const char*>(&command), 1);
					os.write(reinterpret_cast<const char*>(&buffer.back()), 1);
					buffer.clear();
					buffer.push_back(byte);
					command = 0;
				}
				else {
					--command;
				}
			}
			else if (buffer.back() != byte && command >= 0) {
				if (command == 127) {
					os.write(reinterpret_cast<const char*>(&command), 1);
					for (auto it = buffer.begin(); it != buffer.end(); ++it) {
						os.write(reinterpret_cast<const char*>(&(*it)), 1);
					}
					buffer.clear();
					buffer.push_back(byte);
					command = 0;
				}
				else {
					buffer.push_back(byte);
					++command;
				}
			}
			else if (buffer.back() != byte && command < 0) {
				os.write(reinterpret_cast<const char*>(&command), 1);
				os.write(reinterpret_cast<const char*>(&buffer.back()), 1);
				buffer.clear();
				buffer.push_back(byte);
				command = 0;
			}
			else if (buffer.back() == byte && command > 0) {
				buffer.pop_back();
				--command;
				os.write(reinterpret_cast<const char*>(&command), 1);
				for (auto it = buffer.begin(); it != buffer.end(); ++it) {
					os.write(reinterpret_cast<const char*>(&(*it)), 1);
				}
				buffer.clear();
				buffer.push_back(byte);
				command = -1;
			}
		}
		os.write(reinterpret_cast<const char*>(&command), 1);
		if (command < 0) {
			os.write(reinterpret_cast<const char*>(&buffer.back()), 1);
		}
		else if (command >= 0) {
			for (auto it = buffer.begin(); it != buffer.end(); ++it) {
				os.write(reinterpret_cast<const char*>(&(*it)), 1);
			}
		}
		int8_t eod = -128;
		os.write(reinterpret_cast<const char*>(&eod), 1);
	}

	void decompress() {
		while (is.read(reinterpret_cast<char*>(&command), 1)) {
			if (command == -128) {
				return;
			}
			else if (command < 0) {
				is.read(reinterpret_cast<char*>(&byte), 1);
				for (uint8_t i = 0; i < 1 - command; ++i) {
					os.write(reinterpret_cast<const char*>(&byte), 1);
				}
			}
			else if (command >= 0) {
				for (uint8_t i = 0; i < command + 1; ++i) {
					is.read(reinterpret_cast<char*>(&byte), 1);
					os.write(reinterpret_cast<const char*>(&byte), 1);
				}
			}
		}
	}
};

int main(int argc, char* argv[]) {
	if (argc != 4)
		syntax("packbits [c|d] <input file> <output file>");

	std::string option = argv[1];
	std::string input_filename = argv[2];
	std::string output_filename = argv[3];

	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);
	
	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		error("cannot open file " + output_filename);

	packbits packbits(is, os);

	if (option == "c")
		packbits.compress();
	else if (option == "d")
		packbits.decompress();
	else
		syntax("packbits [c|d] <input file> <output file>");

	return EXIT_SUCCESS;
}