#include "lzs.h"

void lzs_decompress(std::istream& is, std::ostream& os) {
	bitreader br(is);
	uint32_t u, offset, length, N;
	std::vector<uint8_t> data;
	size_t i;
	
	while (true) {
		br.read(u, 1);

		if (u == 0) {
			/* Literal Byte */
			br.read(u, 8);
			data.push_back(u);
		}

		else if (u == 1) {
			/* Offset/Length Reference */
			br.read(u, 1);

			if (u == 0) {
				/* Offset >= 128 */
				br.read(offset, 11);
			}

			else if (u == 1) {
				/* Offset < 128 */
				br.read(offset, 7);

				if (offset == 0) {
					/* End Marker */
					break;
				}
			}

			/* Length */
			br.read(length, 2);

			if (length < 3) {
				length += 2;
			}

			else if (length >= 3) {
				br.read(length, 2);

				if (length < 3) {
					length += 5;
				}

				else if (length >= 3) {
					N = 1;
					br.read(length, 4);
					while (length == 15) {
						++N;
						br.read(length, 4);
					}
					length = (N * 15 - 7) + length;
				}
			}

			i = 0;
			while (i < length) {
				data.push_back(data.at(data.size() - offset));
				++i;
			}
		}
	}

	os.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint8_t));
}