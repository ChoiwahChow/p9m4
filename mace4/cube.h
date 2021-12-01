
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <string>
#include <vector>
#include "cell.h"

class Cube {
private:
	size_t domain_size;
	std::string op;
	size_t base;
	int    cell_id;
	int    cell_value;

public:
	Cube(size_t domain_size, Cell Cells);
	virtual ~Cube();

	int value(int id);
};


#endif
