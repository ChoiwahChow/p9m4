
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <string>
#include <vector>
#include "cell.h"

class Cube {
public:
	std::vector<size_t> cell_ids;
private:
	std::vector<int>    cell_values;
	size_t          order;
	Cell            Cells;
	bool            initialized;

public:
	Cube(size_t domain_size, Cell Cells);
	virtual ~Cube();

	int value(size_t depth, size_t id);
	void print_ordered_cells(int number_of_cells);
	int next_id(size_t depth);
};


#endif
