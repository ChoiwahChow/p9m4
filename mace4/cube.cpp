
#include <iostream>
#include <fstream>
#include <algorithm>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells): initialized(false), order(domain_size), Cells(Cells) {
	std::vector<size_t> t_b2 = {0,
								order+1, 1, order,
								2*order+2, 2, 2*order, order+2, 2*order+1,
							    3*order+3, 3, 3*order, order+3, 3*order+1, 2*order+3, 3*order+2,
								4*order+4, 4, 4*order, order+4, 4*order+1, 2*order+4, 4*order+2, 3*order+4, 4*order+3,
								5*order+5, 5, 5*order, order+5, 5*order+1, 2*order+5, 5*order+2, 3*order+5, 5*order+3, 4*order+5, 5*order+4,
								6*order+6, 6, 6*order, order+6, 6*order+1, 2*order+6, 6*order+2, 3*order+6, 6*order+3, 4*order+6, 6*order+4, 5*order+6, 6*order+5,
								7*order+7};
	std::vector<size_t> t_u1_b2 = {0, order,
								   1, 2*order+1, order+1, 2*order,
								   2, 3*order+2, order+2, 3*order, 2*order+2, 3*order+1,
								   3, 4*order+3, order+3, 4*order, 2*order+3, 4*order+1, 3*order+3, 4*order+2,
								   4, 5*order+4, order+4, 5*order, 2*order+4, 5*order+1, 3*order+4, 5*order+2, 4*order+4, 5*order+3,
								   5, 6*order+5};

	std::vector<size_t>& t = t_b2;
	cell_values.insert(cell_values.end(), *max_element(t.begin(), t.end())+1, -1);

	ifstream config("cube.config");
	if (!config.is_open()) {
		cell_ids = t;
		return;
	}
	int cell_value;
	config >> cell_value;
	int pos = 0;
	while (!config.eof()) {
		cell_ids.push_back(t[pos]);
		cell_values[t[pos++]] = cell_value;
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
