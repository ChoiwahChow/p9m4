
#include <iostream>
#include <fstream>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells): domain_size(domain_size), op("*"), base(0), cell_id(-1), cell_value(-1) {
	ifstream config("cube.config");
	config >> cell_id;
	config >> cell_value;
	config.close();
	std::cout << "debug Cube***********************cell_id = " << cell_id << ", cell value = " << cell_value << std::endl;
	if (cell_id < 0)
		return;
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
	cell_id += base;
}


int
Cube::value(int id) {
	if (cell_id != id)
		return -1;
	else
		return cell_value;
}
