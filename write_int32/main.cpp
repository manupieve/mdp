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

int main(int argc, char* argv[]) {
	if (argc != 3)
		syntax("write_int32 <filein.txt> <fileout.bin>");

	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	std::ifstream is(input_filename);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		error("cannot open file " + output_filename);

	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is),
		std::istream_iterator<int32_t>()
	};

	for (const auto& x : v)
		os.write(reinterpret_cast<const char*>(&x), sizeof(x));

	return EXIT_SUCCESS;
}