#include "base64.h"

int main() {
	std::string output = base64_decode("UHJldHR5IGxvbmcgdGV4dCB3aGljaCByZXF1aXJlcyBtb3JlIHRoYW4gNzYgY2hhcmFjdGVycyB0byBlbmNvZGUgaXQgY29tcGxldGVseS4=");
	return EXIT_SUCCESS;
}