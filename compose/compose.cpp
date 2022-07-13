#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>

using rgba = std::array<uint8_t, 4>;

class mat {
	int rows_;
	int cols_;
	std::vector<rgba> data_;

public:
	mat(int rows = 0, int cols = 0) {
		resize(rows, cols);
	}

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	int rows() const {
		return rows_;
	}

	int cols() const {
		return cols_;
	}

	rgba& operator()(int r, int c) {
		return data_[r * cols_ + c];
	}

	const rgba& operator()(int r, int c) const {
		return data_[r * cols_ + c];
	}
};

void update_output_image_size(const mat& input_image, mat& output_image, const int x, const int y) {
	if (output_image.rows() < y + input_image.rows())
		output_image.resize(y + input_image.rows(), output_image.cols());
	if (output_image.cols() < x + input_image.cols())
		output_image.resize(output_image.rows(), x + input_image.cols());
}

void load_pam(const std::string& input_filename, mat& input_image, mat& output_image, const int x, const int y) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		exit(EXIT_FAILURE);

	std::string magic_number;
	is >> magic_number;
	if (magic_number != "P7")
		exit(EXIT_FAILURE);

	std::string width_token, height_token;
	int width, height;
	is >> width_token >> width >> height_token >> height;
	if (width_token != "WIDTH" || height_token != "HEIGHT")
		exit(EXIT_FAILURE);
	if (width < 1 || height < 1)
		exit(EXIT_FAILURE);

	std::string depth_token;
	int depth;
	is >> depth_token >> depth;
	if (depth_token != "DEPTH")
		exit(EXIT_FAILURE);
	if (depth != 3 && depth != 4)
		exit(EXIT_FAILURE);

	std::string maxval_token;
	int maxval;
	is >> maxval_token >> maxval;
	if (maxval_token != "MAXVAL")
		exit(EXIT_FAILURE);
	if (maxval != 255)
		exit(EXIT_FAILURE);

	std::string tupltype_token, tupltype;
	is >> tupltype_token >> tupltype;
	if (tupltype_token != "TUPLTYPE")
		exit(EXIT_FAILURE);
	if (tupltype != "RGB" && tupltype != "RGB_ALPHA")
		exit(EXIT_FAILURE);

	std::string endhdr;
	is >> endhdr;
	if (endhdr != "ENDHDR")
		exit(EXIT_FAILURE);

	if (is.get() != '\n')
		exit(EXIT_FAILURE);

	input_image.resize(height, width);

	update_output_image_size(input_image, output_image, x, y);

	for (int r = 0; r < input_image.rows(); ++r) {
		for (int c = 0; c < input_image.cols(); ++c) {
			if (depth == 3) {
				input_image(r, c)[0] = is.get();
				input_image(r, c)[1] = is.get();
				input_image(r, c)[2] = is.get();
				input_image(r, c)[3] = 255;
			}
			else if (depth == 4) {
				input_image(r, c)[0] = is.get();
				input_image(r, c)[1] = is.get();
				input_image(r, c)[2] = is.get();
				input_image(r, c)[3] = is.get();
			}
		}
	}

	if (is.get() != EOF)
		exit(EXIT_FAILURE);
}

void compose(mat& output_image, const mat& input_image, const int x, const int y) {
	for (int r = 0; r < output_image.rows(); ++r) {
		for (int c = 0; c < output_image.cols(); ++c) {
			if (r >= y && c >= x && r < y + input_image.rows() && c < x + input_image.cols()) {
				double red_a = double(input_image(r - y, c - x)[0]);
				double green_a = double(input_image(r - y, c - x)[1]);
				double blue_a = double(input_image(r - y, c - x)[2]);
				double alpha_a = double(input_image(r - y, c - x)[3]) / (255.0);
				
				double red_b = double(output_image(r, c)[0]);
				double green_b = double(output_image(r, c)[1]);
				double blue_b = double(output_image(r, c)[2]);
				double alpha_b = double(output_image(r, c)[3]) / (255.0);

				double red_o = (red_a * alpha_a + red_b * alpha_b * (1.0 - alpha_a)) / (alpha_a + alpha_b * (1.0 - alpha_a));
				double green_o = (green_a * alpha_a + green_b * alpha_b * (1.0 - alpha_a)) / (alpha_a + alpha_b * (1.0 - alpha_a));
				double blue_o = (blue_a * alpha_a + blue_b * alpha_b * (1.0 - alpha_a)) / (alpha_a + alpha_b * (1.0 - alpha_a));
				double alpha_o = alpha_a + alpha_b * (1.0 - alpha_a);

				output_image(r, c)[0] = uint8_t(red_o);
				output_image(r, c)[1] = uint8_t(green_o);
				output_image(r, c)[2] = uint8_t(blue_o);
				output_image(r, c)[3] = uint8_t(alpha_o * 255.0);
			}
		}
	}
}

void save_pam(const std::string& output_filename, const mat& output_image) {
	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		exit(EXIT_FAILURE);

	os << "P7\nWIDTH " << output_image.cols() << "\nHEIGHT " << output_image.rows() << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";

	for (int r = 0; r < output_image.rows(); ++r) {
		for (int c = 0; c < output_image.cols(); ++c) {
			os.put(output_image(r, c)[0]);
			os.put(output_image(r, c)[1]);
			os.put(output_image(r, c)[2]);
			os.put(output_image(r, c)[3]);
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc < 3)
		return EXIT_FAILURE;

	std::string argument, input_filename, output_filename;
	std::vector<mat> images;
	std::vector<int> x, y;
	mat output_image;
	
	for (int i = 1; i < argc; ++i) {
		if (i == 1) {
			output_filename = argv[i];
			output_filename += ".pam";
		}
		else {
			argument = argv[i];
			if (argument == "-p") {
				++i;
				x.push_back(std::stoi(argv[i]));
				++i;
				y.push_back(std::stoi(argv[i]));
				++i;
				input_filename = argv[i];
				input_filename += ".pam";
			}
			else {
				x.push_back(0);
				y.push_back(0);
				input_filename = argv[i];
				input_filename += ".pam";
			}
			mat input_image;
			load_pam(input_filename, input_image, output_image, x.back(), y.back());
			images.push_back(input_image);
		}
	}

	for (int i = 0; i < images.size(); ++i)
		compose(output_image, images[i], x[i], y[i]);

	save_pam(output_filename, output_image);
	
	return EXIT_SUCCESS;
}