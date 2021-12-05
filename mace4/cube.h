
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <string>
#include <vector>
#include "cell.h"

class Cube {
private:
	std::vector<int> cell_values;
	bool             initialized;

public:
	Cube();
	virtual ~Cube();

	int value(size_t depth);
};


#endif
