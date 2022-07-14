#include "pbm.h"

int main() {
	BinaryImage bimg;
	bimg.ReadFromPBM("im1.pbm");
	Image im;
	im = BinaryImageToImage(bimg);
	
	return EXIT_SUCCESS;
}