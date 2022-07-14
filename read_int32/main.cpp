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

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

int main(int argc, char* argv[]) {
	if (argc != 3)
		syntax("read_int32 <filein.bin> <fileout.txt>");

	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename);
	if (!os)
		error("cannot open file " + output_filename);

	int32_t val;
	std::vector<int32_t> v;

	while (raw_read(is, val)) {
		v.push_back(val);
	}

	for (const auto& x : v) {
		os << x << '\n';
	}

	return EXIT_SUCCESS;
}