#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "mat.h"
#include "ppm.h"

std::string JSON(const std::string& filename);

int main(void) {
	std::cout << JSON("facolta.ppm") << std::endl;
	return EXIT_SUCCESS;
}