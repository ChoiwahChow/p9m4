
#include <iostream>
#include <fstream>
#include <algorithm>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells): initialized(false), order(domain_size), Cells(Cells) {
	cell_values.insert(cell_values.end(), Number_of_cells, -1);
	ifstream config("cube.config");
	if (!config.is_open()) {
		for (size_t idx=0; idx<Number_of_cells; idx++)
			cell_ids.push_back(Ordered_cells[idx]->get_id());
		return;
	}
	int cell_value;
	config >> cell_value;
	int pos = 0;
	while (!config.eof()) {
		cell_ids.push_back(Ordered_cells[pos]->get_id());
		cell_values[Ordered_cells[pos++]->get_id()] = cell_value;
		std::cout << cell_value << " ";
		config >> cell_value;
	}
	config.close();
	initialized = true;

	std::cout << "\ndebug Cube*********************** max_depth = " << cell_ids.size() << std::endl;
}

int
Cube::value(size_t depth, size_t id) {
	if (initialized && cell_ids.size() > 0) {
		std::cout << "Debug, Cube::value, incoming id " << id << std::endl;
		while (cell_ids.size() > 0 && id != cell_ids[0]) {
			if (Cells[cell_ids[0]].get_value() != cell_values[cell_ids[0]]) {
				std::cout << "Debug, Cube::value, Mis-matched cell id =" << cell_ids[0] << ", cell value = " << cell_values[cell_ids[0]]
						  << "  vs  " << Cells[cell_ids[0]].get_value() << std::endl;
				cell_ids.erase(cell_ids.begin());
				return order+1;
			}
			std::cout << "Debug, Cube::value, matched, cell id = "  << cell_ids[0] << std::endl;
			cell_ids.erase(cell_ids.begin());
		}
		if (cell_ids.size() > 0 && id == cell_ids[0]) {
			cell_ids.erase(cell_ids.begin());
		}
		if (id < cell_values.size())
			return cell_values[id];
		else
			return -1;
	}
	return -1;
}
