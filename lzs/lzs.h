#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

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
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	explicit operator bool() { return bool(is_); }

	bool operator!() { return !is_; }
};

void lzs_decompress(std::istream& is, std::ostream& os);