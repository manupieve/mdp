#include <string>
#include <iostream>
#include <fstream>
#include <vector>

template<typename T>
class mat {
	size_t rows_;
	size_t cols_;
	size_t channels_;
	std::vector<T> data_;

public:
	mat(size_t rows = 0, size_t cols = 0, size_t channels = 0) : rows_(rows), cols_(cols), channels_(channels), data_(rows*cols*channels) {}

	size_t rows() const {
		return rows_;
	}

	size_t cols() const {
		return cols_;
	}

	size_t channels() const {
		return channels_;
	}

	size_t size() const {
		return rows_ * cols_ * channels_;
	}

	size_t rawsize() const {
		return rows_ * cols_ * channels_ * sizeof(T);
	}

	char* rawdata() {
		return reinterpret_cast<char*>(&data_[0]);
	}

	const char* rawdata() const {
		return reinterpret_cast<const char*>(&data_[0]);
	}

	void resize(size_t rows, size_t cols, size_t channels) {
		rows_ = rows;
		cols_ = cols;
		channels_ = channels;
		data_.resize(rows * cols * channels);
	}

	T& operator[](size_t i) {
		return data_[i];
	}

	const T& operator[](size_t i) const {
		return data_[i];
	}

	T& operator()(size_t r, size_t c) {
		return data_[r * cols_ + c];
	}

	const T& operator()(size_t r, size_t c) const {
		return data_[r * cols_ + c];
	}
};

bool read_pam(const std::string& filename, mat<uint8_t>& image) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string magic_number;
	std::string token;
	size_t width, height, depth, maxval;
	std::string tupltype;
	bool have_width, have_height, have_depth, have_maxval, have_tupltype;
	have_width = have_height = have_depth = have_maxval = have_tupltype = false;

	is >> magic_number;
	if (magic_number != "P7") {
		return false;
	}

	while (is >> token) {
		if (token == "#") {
			std::string dump;
			std::getline(is, dump);
		}
		if (token == "WIDTH") {
			is >> width;
			have_width = true;
		}
		if (token == "HEIGHT") {
			is >> height;
			have_height = true;
		}
		if (token == "DEPTH") {
			is >> depth;
			have_depth = true;
		}
		if (token == "MAXVAL") {
			is >> maxval;
			have_maxval = true;
		}
		if (token == "TUPLTYPE") {
			is >> tupltype;
			have_tupltype = true;
		}
		if (token == "ENDHDR") {
			if (is.get() != '\n') {
				return false;
			}
			break;
		}
	}

	if (!have_width || !have_height || !have_depth || !have_maxval || !have_tupltype) {
		return false;
	}

	if (depth != 3 || maxval != 255 || tupltype != "RGB") {
		return false;
	}

	image.resize(height, width, depth);

	is.read(image.rawdata(), image.rawsize());

	return true;
}

bool write_pam(const std::string& filename, const mat<uint8_t>& image) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}

	os << "P7\n";
	os << "WIDTH " << image.cols() << "\n";
	os << "HEIGHT " << image.rows() << "\n";
	os << "DEPTH 1\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE GRAYSCALE\n";
	os << "ENDHDR\n";

	os.write(image.rawdata(), image.rawsize());

	return true;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		return EXIT_FAILURE;
	}

	std::string filename = argv[1];
	mat<uint8_t> image;

	if (!read_pam(filename, image)) {
		return EXIT_FAILURE;
	}

	mat<uint8_t> image_R(image.rows(), image.cols(), 1);
	mat<uint8_t> image_G(image.rows(), image.cols(), 1);
	mat<uint8_t> image_B(image.rows(), image.cols(), 1);

	size_t i = 0;
	for (size_t r = 0; r < image.rows(); ++r) {
		for (size_t c = 0; c < image.cols(); ++c) {
			image_R(r, c) = image[i];
			image_G(r, c) = image[i + 1];
			image_B(r, c) = image[i + 2];
			i = i + 3;
		}
	}

	if (!write_pam("image_R.pam", image_R)) {
		return EXIT_FAILURE;
	}
	if (!write_pam("image_G.pam", image_G)) {
		return EXIT_FAILURE;
	}
	if (!write_pam("image_B.pam", image_B)) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}