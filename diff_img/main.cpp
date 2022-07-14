#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

template<typename T>
class mat {
	size_t rows_;
	size_t cols_;
	std::vector<T> data_;

public:
	mat(size_t rows = 0, size_t cols = 0) : rows_(rows), cols_(cols), data_(rows* cols) {}

	size_t rows() const {
		return rows_;
	}

	size_t cols() const {
		return cols_;
	}

	size_t size() const {
		return rows_ * cols_;
	}

	size_t rawsize() const {
		return rows_ * cols_ * sizeof(T);
	}

	char* rawdata() {
		return reinterpret_cast<char*>(&data_[0]);
	}

	const char* rawdata() const {
		return reinterpret_cast<const char*>(&data_[0]);
	}

	void resize(size_t rows, size_t cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
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

bool read_pam(const std::string& filename, mat<uint8_t>& img) {
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

	if (depth != 1 || maxval != 255 || tupltype != "GRAYSCALE") {
		return false;
	}

	img.resize(height, width);

	is.read(img.rawdata(), img.rawsize());

	return true;
}

void print_entropy_original(const mat<uint8_t>& img) {
	std::vector<uint64_t> count(256, 0);

	for (size_t r = 0; r < img.rows(); ++r)
		for (size_t c = 0; c < img.cols(); ++c)
			++count[img(r, c)];

	double H = 0.0;

	for (size_t i = 0; i < 256; ++i) {
		double p = (double)count[i] / img.size();
		if (p > 0)
			H -= p * log2(p);
	}

	std::cout << "Entropy Original: " << H << std::endl;
}

void build_difference_matrix(const mat<uint8_t>& I, mat<int16_t>& D) {
	D.resize(I.rows(), I.cols());

	for (size_t x = 0; x < D.rows(); ++x) {
		for (size_t y = 0; y < D.cols(); ++y) {
			if (y > 0)
				D(x, y) = I(x, y) - I(x, y - 1);
			if (y == 0 && x > 0)
				D(x, y) = I(x, y) - I(x - 1, y);
			if (x == 0 && y == 0)
				D(x, y) = I(x, y);
		}
	}
}

void build_difference_image(const mat<int16_t>& matrix, mat<uint8_t>& image) {
	image.resize(matrix.rows(), matrix.cols());
	
	for (size_t r = 0; r < image.rows(); ++r)
		for (size_t c = 0; c < image.cols(); ++c)
			image(r, c) = matrix(r, c) / 2 + 128;
}

bool write_pam(const std::string& filename, const mat<uint8_t>& img) {
	std::ofstream os(filename, std::ios::binary);
	if (!os) {
		return false;
	}

	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 1\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE GRAYSCALE\n";
	os << "ENDHDR\n";

	os.write(img.rawdata(), img.rawsize());

	return true;
}

void print_entropy_difference(const mat<int16_t>& matrix) {
	std::unordered_map<int16_t, uint64_t> count;

	for (size_t r = 0; r < matrix.rows(); ++r)
		for (size_t c = 0; c < matrix.cols(); ++c)
			++count[matrix(r, c)];

	double H = 0.0;

	for (auto it = count.begin(); it != count.end(); ++it) {
		double p = (double)it->second / matrix.size();
		if (p > 0)
			H -= p * log2(p);
	}

	std::cout << "Entropy Difference: " << H << std::endl;
}

int main() {
	std::string input_filename = "frog.pam";
	mat<uint8_t> original_image;

	if (!read_pam(input_filename, original_image))
		return EXIT_FAILURE;

	print_entropy_original(original_image);

	mat<int16_t> difference_matrix;

	build_difference_matrix(original_image, difference_matrix);

	std::string output_filename = "output.pam";
	mat<uint8_t> difference_image;

	build_difference_image(difference_matrix, difference_image);

	if (!write_pam(output_filename, difference_image))
		return EXIT_FAILURE;

	print_entropy_difference(difference_matrix);

	return EXIT_SUCCESS;
}