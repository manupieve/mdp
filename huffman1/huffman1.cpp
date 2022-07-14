#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unordered_map>

void syntax(const std::string& msg) {
	std::cout << "Syntax: " << msg << '\n';
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "Error: " << msg << '\n';
	exit(EXIT_FAILURE);
}

struct node {
	uint32_t count;
	uint8_t sym;
	uint32_t len = 0;
	uint32_t code = 0;
	node* left;
	node* right;

	node() : count(0), sym(0), left(nullptr), right(nullptr) {}
	node(node left, node right) : count(left.count + right.count), sym(0), left(new node(left)), right(new node(right)) {}
	~node() {
		if (left == nullptr)
			delete left;
		if (right == nullptr)
			delete right;
	}
};

struct row {
	uint8_t sym;
	uint32_t len;
	uint32_t code;

	row(uint8_t sym, uint32_t len, uint32_t code) : sym(sym), len(len), code(code) {}
};

struct table {
	std::unordered_map<uint8_t, row> rows;

	void add_row(uint8_t sym, uint32_t len, uint32_t code) {
		rows.insert(std::make_pair(sym, row(sym, len, code)));
	}
};

class bitwriter {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

public:

	bitwriter(std::ostream& os) : os_(os) {}

	~bitwriter() { flush(); }

	std::ostream& write(uint32_t u, uint8_t n) {
		while (n-- > 0) {
			uint8_t bit = (u >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, uint8_t n) {
		return write(u, n);
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}
};

bool compare(node a, node b) {
	return a.count < b.count;
}

std::vector<node> create_nodes(std::ifstream& is) {
	uint8_t val;
	std::vector<node> count(256);
	while (is.read(reinterpret_cast<char*>(&val), 1)) {
		++count[val].count;
		count[val].sym = val;
	}

	std::vector<node> nodes;
	for (auto it = count.begin(); it != count.end(); ++it) {
		if ((*it).count > 0) {
			nodes.push_back(*it);
		}
	}

	std::sort(nodes.begin(), nodes.end(), compare);

	return nodes;
}

node* create_tree(std::vector<node>& nodes) {
	if (nodes.size() == 1)
		return new node(nodes.front());

	node left = nodes.front();
	nodes.erase(nodes.begin());
	node right = nodes.front();
	nodes.erase(nodes.begin());

	node parent_node = node(left, right);
	nodes.push_back(parent_node);
	std::sort(nodes.begin(), nodes.end(), compare);

	return create_tree(nodes);
}

void code_nodes(node* n) {
	if (n->left != nullptr && n->right != nullptr) {
		n->left->len = n->len + 1;
		n->left->code = n->code << 1;

		n->right->len = n->len + 1;
		n->right->code = (n->code << 1) | 1;

		code_nodes(n->left);
		code_nodes(n->right);
	}
	else
		return;
}

void create_table(node* n, table& t) {
	if (n->left == nullptr && n->right == nullptr) {
		t.add_row(n->sym, n->len, n->code);
		return;
	}
	else {
		create_table(n->left, t);
		create_table(n->right, t);
	}
}

void write_file(std::ofstream& os, table& t, node* root, std::ifstream& is) {
	os.write("HUFFMAN1", 8);
	
	char num_table_entries = (char)t.rows.size();
	os.write(&num_table_entries, 1);

	bitwriter bw(os);
	for (auto it = t.rows.begin(); it != t.rows.end(); ++it) {
		bw(it->second.sym, 8);
		bw(it->second.len, 5);
		bw(it->second.code, it->second.len);
	}

	bw(root->count, 32);

	is.clear();
	is.seekg(0);
	char val;
	while (is.read(&val, 1)) {
		bw(t.rows.find(val)->second.code, t.rows.find(val)->second.len);
	}
}

void compress(const std::string& input_filename, const std::string& output_filename) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		error("cannot open file " + output_filename);

	std::vector<node> nodes = create_nodes(is);

	node* root = create_tree(nodes);

	code_nodes(root);

	table t;
	create_table(root, t);

	write_file(os, t, root, is);
}

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::istream& is_;

	int read_bit() {
		if (nbits_ == 0) {
			if (!raw_read(is_, buffer_)) {
				return EOF;
			}
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

public:

	bitreader(std::istream& is) : is_(is) {}

	std::istream& read(uint32_t& u, uint8_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	explicit operator bool() { return bool(is_); }

	bool operator!() { return !is_; }
};

char read_symbol(const table& t, bitreader& br) {
	uint32_t u;
	uint32_t len = 0;
	uint32_t code = 0;
	while (true) {
		br.read(u, 1);
		++len;
		code = (code << 1) | u;

		for (auto it = t.rows.begin(); it != t.rows.end(); ++it) {
			if (len == it->second.len && code == it->second.code)
				return it->second.sym;
		}
	}
}

void read_file(std::ifstream& is, table& t, std::ofstream& os) {
	char magic_number[8];
	is.read(magic_number, 8);

	char num_table_entries;
	is.read(&num_table_entries, 1);

	bitreader br(is);
	for (size_t i = 0; i < num_table_entries; ++i) {
		uint32_t sym;
		br.read(sym, 8);

		uint32_t len;
		br.read(len, 5);

		uint32_t code;
		br.read(code, len);

		t.add_row(sym, len, code);
	}

	uint32_t count;
	br.read(count, 32);

	for (size_t i = 0; i < count; ++i) {
		char sym = read_symbol(t, br);
		os.write(&sym, 1);
	}
}

void decompress(const std::string& input_filename, const std::string& output_filename) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is)
		error("cannot open file " + input_filename);

	std::ofstream os(output_filename, std::ios::binary);
	if (!os)
		error("cannot open file " + output_filename);

	table t;
	read_file(is, t, os);
}

int main(int argc, char* argv[]) {
	if (argc != 4)
		syntax("huffman1 [c|d] <input file> <output file>");

	std::string input_filename = argv[2];
	std::string output_filename = argv[3];

	if (argv[1][0] == 'c')
		compress(input_filename, output_filename);
	else if (argv[1][0] == 'd')
		decompress(input_filename, output_filename);
	else
		syntax("huffman1 [c|d] <input file> <output file>");
	
	return EXIT_SUCCESS;
}