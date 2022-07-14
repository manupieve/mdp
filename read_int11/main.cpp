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

class bitreader {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::istream& is_;

	int read_bit() {
		if (nbits_ == 0) {
			if (!raw_read(is_, buffer_)) {
				return EOF;
			}
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

public:

	bitreader(std::istream& is) : is_(is) {}

	std::istream& read(uint32_t& u, uint8_t n) {
		u = 0;
		while (n --> 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	explicit operator bool() { return bool(is_); }

	bool operator!() { return !is_; }
};

int main(int argc, char* argv[]) {
	if (argc != 3)
		syntax("read_int11 <filein.bin> <fileout.txt>");

	std::string input_filename = argv[1];
	std::string output_filename = argv[2];

	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename);
	if (!os)
		error("cannot open file " + output_filename);

	bitreader br(is);
	uint32_t u;
	std::vector<int32_t> v;

	while (br.read(u, 11)) {
		if (u & 0b10000000000) {
			u |= 0xfffff800;
		}
		v.push_back(u);
	}

	copy(v.begin(), v.end(), std::ostream_iterator<int32_t>(os, "\n"));

	return EXIT_SUCCESS;
}