#include "pcx.h"

int main(void) {
	std::string filename = "gatto_grigio_8bit.pcx";
	mat<vec3b> img;
	load_pcx(filename, img);
	return EXIT_SUCCESS;
}