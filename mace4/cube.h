
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <string>
#include <vector>
#include "cell.h"

class Cube {
private:
	std::vector<int>     cell_values;
	size_t           order;
	size_t           max_depth;
	Cell             Cells;
	Cell*			 Ordered_cells;
	bool             initialized;

public:
	Cube(size_t domain_size, Cell Cells, Cell*& Ordered_cells);
	virtual ~Cube();

	int value(size_t depth, size_t id);
	void reOrderCells(int Number_of_cells);
};


#endif
