
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <string>
#include <vector>
#include "cell.h"

class Cube {
public:
	std::vector<size_t> cell_ids;
private:
	std::vector<int>  cell_values;
	size_t            order;
	size_t            branch_depth;
	Cell              Cells;
	bool              initialized;
	size_t            current_pos;  // current position of the search in Ordered_cells
	size_t            max_pos;      // max value of current position
	std::vector<int>  last_printed;

public:
	Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells);
	virtual ~Cube();

	int value(size_t depth, size_t id);
	void print_new_cube(int cube_length);
	// void print_ordered_cells(int number_of_cells) const;
};


#endif
