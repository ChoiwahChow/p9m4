
#include <iostream>
#include <fstream>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells): initialized(false), order(domain_size), Cells(Cells), max_depth(3) {
	ifstream config("cube.config");
	if (!config.is_open())
		return;
	std::vector<size_t> cell_ids = {0, 1, order, order+1};
	//cell_ids.push_back(order);
	//cell_ids.push_back(order+1);

	cell_values.insert(cell_values.end(), order+1+1, -1);
	std::cout << "debug Cube ************** values = ";
	int cell_value;
	config >> cell_value;
	int pos = 0;
	while (!config.eof()) {
		cell_values[cell_ids[pos++]] = cell_value;
		std::cout << cell_value << " ";
		config >> cell_value;
	}
	config.close();
	initialized = true;
	std::cout << "\ndebug Cube*********************** max_depth = " << cell_values.size() - 1 << std::endl;
}


int
Cube::value(size_t depth, size_t id) {
	if (initialized && max_depth >= depth && id < cell_values.size()) {
		if (cell_values[id] == -1) {
			std::cout << "depth " << depth << " propagated id=" << id << " value = " << Cells[(depth / 2) * order + depth % 2].get_value() << std::endl;
		}
		return cell_values[id];
	}
	if (depth <= max_depth) {
		std::cout << "depth " << depth << " propagated id=" << id << " value = " << Cells[(depth / 2) * order + depth % 2].get_value() << std::endl;
	}
	return -1;
}
