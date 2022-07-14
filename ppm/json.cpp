#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "mat.h"
#include "ppm.h"

bool LoadPPM(const std::string& filename, mat<vec3b>& img);
void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b);
void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded);
std::string Base64Encode(const std::vector<uint8_t>& v);

std::string JSON(const std::string& filename) {
	mat<vec3b> img;
	if (!LoadPPM(filename, img))
		return "{}";

	mat<uint8_t> img_r, img_g, img_b;
	SplitRGB(img, img_r, img_g, img_b);

	std::vector<uint8_t> encoded_r, encoded_g, encoded_b;
	PackBitsEncode(img_r, encoded_r);
	PackBitsEncode(img_g, encoded_g);
	PackBitsEncode(img_b, encoded_b);

	std::string json;
	json += "{\n";
	json += "\t\"width\" : " + std::to_string(img.cols()) + ",\n";
	json += "\t\"height\" : " + std::to_string(img.rows()) + ",\n";
	json += "\t\"red\" : \"" + Base64Encode(encoded_r) + "\",\n";
	json += "\t\"green\" : \"" + Base64Encode(encoded_g) + "\",\n";
	json += "\t\"blue\" : \"" + Base64Encode(encoded_b) + "\"\n";
	json += "}";

	return json;
}