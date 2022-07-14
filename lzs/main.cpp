#include "lzs.h"

int main() {
	std::ifstream is("bibbia.txt.lzs", std::ios::binary);
	if (!is)
		return EXIT_FAILURE;
	std::ofstream os("bibbia.txt", std::ios::binary);
	if (!os)
		return EXIT_FAILURE;

	lzs_decompress(is, os);

	return EXIT_SUCCESS;
}