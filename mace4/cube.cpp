
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


	/*
	size_t id = 0;
	bool base_initialized = false;
	while( !base_initialized ) {
		std::string& this_op = Symbol_dataContainer::get_op_symbol(Cells[id].get_sn());
		if (this_op == op) {
			base = Cells[id].get_base();
			base_initialized = true;
		}
		id ++;
	}
	int cell_id;
	int cell_value;
	config >> cell_id;
	while (cell_id >= 0) {
		cell_ids.push_back(cell_id + base);
		config >> cell_value;
		cell_values.push_back(cell_value);
		config >> cell_id;
	}
	config.close();
	std::cout << "debug Cube***********************base = " << base << ", number of cells = " << cell_values.size() << std::endl;
	if (cell_ids.size() == 0)
		return;
	cell_id += base;
	*/
}


int
Cube::value(size_t depth) {
	if (initialized && cell_values.size() > depth)
		return -1;
	return cell_values[depth];
	/*
	for( size_t idx = 0; idx < cell_ids.size(); ++idx ) {
		if (cell_ids[idx] == id)
			return cell_values[idx];
	}
	return -1;
	*/
}
