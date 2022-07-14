#include <string>

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits);

int main(void) {
	lz78encode("test1.txt", "out1.bin", 4);
}