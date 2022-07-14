#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

void syntax(const std::string& msg) {
	std::cout << "Syntax: " << msg << '\n';
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "Error: " << msg << '\n';
	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
	if (argc != 3)
		syntax("frequencies <input file> <output file>");

	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename);
	if (!os)
		error("cannot open file " + output_filename);

	uint8_t val;
	std::vector<uint64_t> count(256, 0);

	while (is.read(reinterpret_cast<char*>(&val), 1))
		++count[val];

	for (auto it = count.begin(); it != count.end(); ++it)
		if (*it > 0) {
			os << std::hex << std::setw(2) << std::setfill('0') << std::distance(count.begin(), it);
			os << std::dec << '\t' << *it << '\n';
		}

	return EXIT_SUCCESS;
}