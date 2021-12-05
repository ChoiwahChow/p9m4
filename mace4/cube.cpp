
#include <iostream>
#include <fstream>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(): initialized(false) {
	ifstream config("cube.config");
	if (!config.is_open())
		return;
	std::cout << "debug Cube ************** values = ";
	int cell_value;
	config >> cell_value;
	while (!config.eof()) {
		cell_values.push_back(cell_value);
		std::cout << cell_value << " ";
		config >> cell_value;
	}
	config.close();
	initialized = true;
	std::cout << "\ndebug Cube*********************** max_depth = " << cell_values.size() - 1<< std::endl;
}


int
Cube::value(size_t depth) {
	if (initialized && cell_values.size() > depth)
		return -1;
	return cell_values[depth];
}
