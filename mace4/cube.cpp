
#include <iostream>
#include <fstream>
#include "syms.h"
#include "cube.h"


Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells, Cell*& Ordered_cells): initialized(false), order(domain_size),
		Cells(Cells), Ordered_cells(Ordered_cells), max_depth(3) {
	ifstream config("cube.config");
	if (!config.is_open())
		return;
	std::vector<size_t> cell_ids = {0, order+1, 1, order, 2};
	//cell_ids.push_back(order);
	//cell_ids.push_back(order+1);

	cell_values.insert(cell_values.end(), order+1+1, -1);
	std::cout << "debug Cube ************** values = ";
	int cell_value;
	config >> cell_value;
	int pos = 0;
	max_depth = -1;
	while (!config.eof()) {
		cell_values[cell_ids[pos++]] = cell_value;
		std::cout << cell_value << " ";
		config >> cell_value;
		max_depth++;
	}
	config.close();
	initialized = true;
	std::cout << "\ndebug Cube*********************** max_depth = " << max_depth << std::endl;
	reOrderCells(8);
}


int
Cube::value(size_t depth, size_t id) {
	std::cout << "debug Cube::value, depth " << depth << ", in-coming id " << id << ", expecting " << Ordered_cells[depth]->get_id() << std::endl;
	if (initialized && max_depth >= depth) {
		int value_to_set = cell_values[depth];
		if (Ordered_cells[depth]->get_id() == id)
			return value_to_set;
		// If control passes this point, propagation has filled the next cell in the search order
		std::cout << "debug Cube::value, prop value " << Cells[Ordered_cells[depth]->get_id()].get_value() << std::endl;
		int propagated_value = Cells[Ordered_cells[depth]->get_id()].get_value();
		std::cout << "debug Cube::value, propagated cell " << Ordered_cells[depth]->get_id()
				<< "(" << Ordered_cells[depth]->get_index(0) << ", " << Ordered_cells[depth]->get_index(1)
				<< " = " << propagated_value << std::endl;
		if (value_to_set == propagated_value)
			return -1;
		else
			return -2; // don't continue
	}
	/*
	if (initialized && max_depth >= depth && id < cell_values.size()) {
		if (cell_values[id] == -1) {
			if (Cells[(depth / 2) * order + depth % 2].has_value())
				std::cout << "debug* depth " << depth << " propagated id=" << id << " value = " << Cells[(depth / 2) * order + depth % 2].get_value() << std::endl;
			else
				std::cout << "debug* cell not assigned, depth " << depth << " propagated id=" << id << std::endl;
		}
		return cell_values[id];
	}
	if (initialized && depth <= max_depth && Cells[(depth / 2) * order + depth % 2].has_value()) {
		std::cout << "debug** depth " << depth << " propagated id=" << id << " value = " << Cells[(depth / 2) * order + depth % 2].get_value() << std::endl;
	}
	*/
	return -1;
}

void
Cube::reOrderCells(int Number_of_cells) {
	std::cout << "debug reOrderCells***********"
			<< Ordered_cells[0]->get_id() << "(" << Ordered_cells[0]->get_index(0) << ", " << Ordered_cells[0]->get_index(1)
			<< ")** " << Ordered_cells[1]->get_id() << "("  << Ordered_cells[1]->get_index(0)<< ", " << Ordered_cells[1]->get_index(1)
			<< ")** " << Ordered_cells[2]->get_id() << "("  << Ordered_cells[2]->get_index(0) << ", " << Ordered_cells[2]->get_index(1) << ")**" << std::endl;
}
