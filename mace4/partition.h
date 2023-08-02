
#ifndef MACE4_PARTITION_H
#define MACE4_PARTITION_H

#include <string>
#include <vector>
#include "cell.h"

class Partition {
private:
	size_t domain_size;
	std::string op;
	size_t base;
	int    num_diag;
	int    num_idempotent;
	Cell   Cells;
	size_t too_few;
	size_t too_many;
	size_t good_continue;
	std::vector<size_t> diagonal;

	int count_diagonal(int el);
	int count_idempotent();

public:
	Partition(size_t domain_size, Cell Cells);
	virtual ~Partition();

	bool good_to_go(Cell Cells, int id, int el);
	bool good_to_continue();
	bool validate_model();
};


#endif
