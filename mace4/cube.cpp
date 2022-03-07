
#include <iostream>
#include <fstream>
#include <algorithm>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells): initialized(false),
		order(domain_size), Cells(Cells), current_pos(0), branch_depth(Number_of_cells+1), max_pos(0) {
	cell_values.insert(cell_values.end(), Number_of_cells, -1);
	ifstream config("cube.config");
	for (size_t idx=0; idx<Number_of_cells; idx++)
		cell_ids.push_back(Ordered_cells[idx]->get_id());
	if (!config.is_open()) {
		return;
	}
	int cell_value;
	config >> cell_value;
	std::cout << "Debug Cube::Cube ";
	while (!config.eof()) {
		std::cout << cell_ids[max_pos] << "|" << cell_value << " ";
		cell_values[cell_ids[max_pos++]] = cell_value;
		config >> cell_value;
	}
	config.close();
	initialized = true;

	std::cout << "\ndebug Cube*********************** max_depth = " << max_pos-1 << std::endl;
    // /* debug
    for (int idx = 0; idx < Number_of_cells && idx < 20; ++idx)
	  std::cout <<  Ordered_cells[idx]->get_symbol() << "|" << Ordered_cells[idx]->get_id() << "  ";
    std::cout << "Debug order_cells ********************" << std::endl;
}

int
Cube::value(size_t depth, size_t id) {
	if (initialized && current_pos < max_pos) {
		std::cout << "Debug, Cube::value, incoming id " << id << std::endl;
		while (id != cell_ids[current_pos] && current_pos < max_pos) {
			if ((Cells[cell_ids[current_pos]].get_value() != cell_values[cell_ids[current_pos]]) && (cell_values[cell_ids[current_pos]] != -1)) {
				std::cout << "Debug, Cube::value, Mis-matched cell id =" << cell_ids[current_pos] << ", cell value = " << cell_values[cell_ids[current_pos]]
						  << "  vs  " << Cells[cell_ids[current_pos]].get_value() << std::endl;
				current_pos++;
				return order+1;
			}
			// std::cout << "Debug, Cube::value, matched, cell id = "  << cell_ids[current_pos] << "  current pos " << current_pos << std::endl;
			current_pos++;
		}
		if (current_pos >= max_pos)
			return -1;
		if (id == cell_ids[current_pos]) {
			current_pos++;
		}
		// std::cout << "Debug value pos" << current_pos << std::endl;
		return cell_values[id];
	}
	return -1;
}

void
Cube::print_new_cube(int cube_length) {
	bool same = true;
	if (last_printed.size() < cube_length)
		last_printed.insert(last_printed.end(), cube_length, -1);
  	for (int idx = 0; idx < cube_length; ++idx) {
  	  if (last_printed[idx] != Cells[cell_ids[idx]].get_value()) {
  		  last_printed[idx] = Cells[cell_ids[idx]].get_value();
  		  same = false;
  	  }
  	}
  	if (same)
  		return;
	/* debug print
  	std::cout << "model: depth " << depth << " cell";
  	for (int idx = 0; idx < print_cubes; ++idx)
      std::cout << " " << splitter.cell_ids[idx] << "|" << Cells[splitter.cell_ids[idx]].get_symbol();
  	std::cout << std::endl;
  	*/
  	std::cout << "cube";
  	for (int idx = 0; idx < cube_length; ++idx)
  	  std::cout << " " << last_printed[idx];
  	std::cout << std::endl;
}



