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

class bitwriter {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

public:

	bitwriter(std::ostream &os) : os_(os) {}

	~bitwriter() { flush(); }

	std::ostream& write(uint32_t u, uint8_t n) {
		while (n --> 0) {
			uint8_t bit = (u >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, uint8_t n) {
		return write(u, n);
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}
};

int main(int argc, char* argv[]) {
	if (argc != 3)
		syntax("write_int11 <filein.txt> <fileout.bin>");

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

	bitwriter bw(os);

	for (const auto& x : v) {
		bw(x, 11);
	}

	return EXIT_SUCCESS;
}