#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

class bitwriter {
	uint8_t buffer_ = 0;
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

	bitwriter(std::ostream& os) : os_(os) {}

	~bitwriter() { flush(); }

	std::ostream& write(uint32_t u, uint8_t n) {
		while (n-- > 0) {
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

void init_dict(std::unordered_map<std::string, size_t>& dictionary) {
	dictionary.clear();
	dictionary[""] = 0;
}

size_t max_used_bits(size_t dict_size) {
	size_t i = 0;

	while (dict_size != 0) {
		++i;
		dict_size = dict_size >> 1;
	}

	return i;
}

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		return false;

	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		return false;

	if (maxbits < 1 || maxbits > 30)
		return false;

	std::string magic_number = "LZ78";
	os << magic_number;

	bitwriter bw(os);
	bw.write(maxbits, 5);

	is.unsetf(std::ios::skipws);

	std::unordered_map<std::string, size_t> dictionary;
	init_dict(dictionary);

	size_t dict_max_size = size_t(std::pow(2, maxbits));

	char c;
	std::string buffer;
	size_t current_index = 0;
	size_t last_inserted_index = 0;
	while (true) {
		is >> c;
		buffer += c;
		if (is.peek() == EOF)
			break;
		auto it = dictionary.find(buffer);
		if (it != dictionary.end()) {
			current_index = it->second;
		}
		else {
			size_t n_bits = max_used_bits(last_inserted_index);
			bw.write(current_index, n_bits);
			bw.write(c, 8);
			if (dictionary.size() == dict_max_size) {
				init_dict(dictionary);
				last_inserted_index = 0;
			}
			else {
				dictionary[buffer] = ++last_inserted_index;
			}
			buffer.clear();
			current_index = 0;
		}
	}

	if (!buffer.empty()) {
		size_t n_bits = max_used_bits(last_inserted_index);
		bw.write(current_index, n_bits);
		bw.write(c, 8);
	}

	is.setf(std::ios::skipws);

	return true;
}