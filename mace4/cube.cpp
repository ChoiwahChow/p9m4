
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

	int cell_value;
	config >> cell_value;
	int pos = 0;
	max_depth = -1;
	while (!config.eof()) {
		cell_values.push_back(cell_value);
		config >> cell_value;
		max_depth++;
	}
	config.close();
	initialized = true;

	std::cout << "\ndebug Cube*********************** max_depth = " << max_depth << std::endl;
	print_ordered_cells(4);
}

int
Cube::next_id(size_t depth) {
	// returns the id in the cube if no conflicts
	// returns -1 if no id fixed by the cube - already passed the cube length
	// returns -2 if no id is possible - the propagated cell value does not fit the cube mandate
	if (initialized && max_depth >= depth) {
		int id = Ordered_cells[depth]->get_id();
		if (!Cells[id].has_value())
			return id;
		else if (Cells[id].get_value() == cell_values[depth])
			return -1;
		else {
			std::cout << "debug Cube::value, propagated cell value conflict id: " << Ordered_cells[depth]->get_id()
					<< " = " << Cells[id].get_value() << " vs cube: " << cell_values[depth] << std::endl;
			return -2;
		}
	}
	return -1;
}


int
Cube::value(size_t depth, size_t id) {
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
	return -1;
}

void
Cube::print_ordered_cells(int number_of_cells) {
	std::cout << "debug print_ordered_cells***********";
	for (int ndx = 0; ndx < number_of_cells; ndx++) {
		std::cout << Ordered_cells[ndx]->get_id() << " (" << Ordered_cells[ndx]->get_index(0) << ", " << Ordered_cells[ndx]->get_index(1) << ") ** ";
	}
	cout << std::endl;
}
