#include "base64.h"

int convert(const char& c) {
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int val = int(alphabet.find(c));

	return val;
}

std::string base64_decode(const std::string& input) {
	std::string output;

	if (input.empty())
		return output;

	uint32_t buffer = 0, count = 0;
	
	for (const auto& c : input) {
		if (c != '=') {
			int val = convert(c);
			if (val != -1) {
				buffer = buffer << 6 | val;
				count = count + 1;
				if (count == 4) {
					output = output + char(buffer >> 16);
					output = output + char(buffer >> 8);
					output = output + char(buffer >> 0);
					count = 0;
				}
			}
		}
	}

	if (count == 2) {
		output = output + char(buffer >> 4);
	}
	if (count == 3) {
		output = output + char(buffer >> 10);
		output = output + char(buffer >> 2);
	}

	return output;
}