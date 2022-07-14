#include <string>
#include <iostream>
#include <fstream>
#include <vector>

template<typename T>
class mat {
	size_t rows_;
	size_t cols_;
	std::vector<T> data_;

public:
	mat(size_t rows = 0, size_t cols = 0) : rows_(rows), cols_(cols), data_(rows*cols) {}

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

void exercise_1() {
	mat<uint8_t> img(256, 256);

	for (size_t r = 0; r < img.rows(); ++r) {
		for (size_t c = 0; c < img.cols(); ++c) {
			img(r, c) = uint8_t(r);
		}
	}

	write_pam("output.pam", img);
}

void exercise_2() {
	mat<uint8_t> img;

	read_pam("frog.pam", img);

	for (size_t r = 0; r < img.rows() / 2; ++r) {
		for (size_t c = 0; c < img.cols(); ++c) {
			std::swap(img(r, c), img(img.rows() - 1 - r, c));
		}
	}

	write_pam("upside_down.pam", img);
}

void exercise_3() {
	mat<uint8_t> img;

	read_pam("frog.pam", img);

	for (size_t r = 0; r < img.rows(); ++r) {
		for (size_t c = 0; c < img.cols() / 2; ++c) {
			std::swap(img(r, c), img(r, img.cols() - 1 - c));
		}
	}

	write_pam("mirrored.pam", img);
}

int main() {
	exercise_1();

	exercise_2();

	exercise_3();
	
	return 0;
}