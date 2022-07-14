#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "mat.h"
#include "ppm.h"

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	std::vector<uint8_t> buffer;
	uint8_t byte = 0;
	int8_t command = 0;

	for (auto it = img.begin(); it != img.end(); ++it) {
		byte = *it;

		if (buffer.size() == 0) {
			buffer.push_back(byte);
		}
		else if (buffer.back() == byte && command <= 0) {
			if (command == -127) {
				encoded.push_back(command);
				encoded.push_back(buffer.back());
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
				encoded.push_back(command);
				for (auto it = buffer.begin(); it != buffer.end(); ++it) {
					encoded.push_back(*it);
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
			encoded.push_back(command);
			encoded.push_back(buffer.back());
			buffer.clear();
			buffer.push_back(byte);
			command = 0;
		}
		else if (buffer.back() == byte && command > 0) {
			buffer.pop_back();
			--command;
			encoded.push_back(command);
			for (auto it = buffer.begin(); it != buffer.end(); ++it) {
				encoded.push_back(*it);
			}
			buffer.clear();
			buffer.push_back(byte);
			command = -1;
		}
	}
	encoded.push_back(command);
	if (command < 0) {
		encoded.push_back(buffer.back());
	}
	else if (command >= 0) {
		for (auto it = buffer.begin(); it != buffer.end(); ++it) {
			encoded.push_back(*it);
		}
	}
	int8_t eod = -128;
	encoded.push_back(eod);
}

std::string Base64Encode(const std::vector<uint8_t>& v) {
	std::vector<uint8_t> input = v;
	std::string output = "";

	if (input.empty())
		return output;

	while ((input.size() % 3) != 0)
		input.push_back(128);

	uint32_t buffer = 0, count = 0;
	int val1 = 0, val2 = 0, val3 = 0, val4 = 0;
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	for (const auto& byte : input) {
		buffer = (buffer << 8) | byte;
		count = count + 1;
		if (count == 3) {
			val1 = ((buffer >> 18) & 63);
			if (val1 >= 0 && val1 <= 63)
				output = output + char(alphabet.at(val1));
			val2 = ((buffer >> 12) & 63);
			if (val2 >= 0 && val2 <= 63)
				output = output + char(alphabet.at(val2));
			val3 = ((buffer >> 6) & 63);
			if (val3 >= 0 && val3 <= 63)
				output = output + char(alphabet.at(val3));
			val4 = ((buffer >> 0) & 63);
			if (val4 >= 0 && val4 <= 63)
				output = output + char(alphabet.at(val4));
			count = 0;
		}
	}

	return output;
}