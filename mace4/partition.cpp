
#include <iostream>
#include <fstream>
#include "syms.h"
#include "partition.h"


Partition::~Partition() {
	// TODO Auto-generated destructor stub
	std::cerr << "debug************* too_few = " << too_few << std::endl;
	std::cerr << "debug************* too_many = " << too_many << std::endl;
	std::cerr << "debug************* good_continue = " << good_continue << std::endl;
}

Partition::Partition(size_t domain_size, Cell Cells): domain_size(domain_size), op("*"), num_diag(4),
		num_idempotent(1), base(0), Cells(Cells), good_continue(0), too_few(0), too_many(0),
		diagonal(std::vector<size_t>(domain_size, 0)) {
	ifstream config("diag.config");
	config >> num_diag;
	config >> num_idempotent;
	config.close();
	std::cout << "debug Partition***********************num_diag = " << num_diag << ", num idempotent = " << num_idempotent << std::endl;
	if (num_diag <= 0)
		return;
	size_t id = 0;
	bool base_initialized = false;
	while( !base_initialized ) {
		std::string& this_op = Symbol_dataContainer::get_op_symbol(Cells[id].get_sn());
		if (this_op == op) {
			base = Cells[id].get_base();
			base_initialized = true;
		}
		id ++;
	}
}

int
Partition::count_idempotent() {
	if (num_idempotent < 0)
		return -1;
	int counter = 0;
	for( int idx = 0; idx < domain_size; ++idx ) {
		size_t diag_pos = base + (idx*domain_size) + idx;
		if (Cells[diag_pos].has_value() && (Cells[diag_pos].get_value() == idx)) {
			counter ++;
		}
	}

	return counter;
}

int
Partition::count_diagonal(int el) {
	if (num_diag <= 0)
		return -1;
	for( int idx = 0; idx < domain_size; ++idx )
		diagonal[idx] = 0;
	if (el > -1)
		diagonal[el] = 1;
	int counter = 0;
	for( int idx = 0; idx < domain_size; ++idx ) {
		size_t diag_pos = base + (idx*domain_size) + idx;
		if (!Cells[diag_pos].has_value())
			continue;
		int diag_el = Cells[diag_pos].get_value();
		if (diagonal[diag_el] == 0) {
			counter ++;
			diagonal[diag_el] += 1;
		}
	}

	return counter;
}

bool
Partition::good_to_continue() {
	if (num_diag <= 0)
		return true;
	for( int idx = 0; idx < domain_size; ++idx )
		diagonal[idx] = 0;
	int counter = 0;
	int idempotent_counter = 0;
	int empty = 0;
	for( int idx = 0; idx < domain_size; ++idx ) {
		size_t diag_pos = base + (idx*domain_size) + idx;
		if (!Cells[diag_pos].has_value()) {
			empty++;
			continue;
		}
		int diag_el = Cells[diag_pos].get_value();
		if (diagonal[diag_el] == 0) {
			counter ++;
			if (counter > num_diag) {
				too_many++;
				return false;
			}
			diagonal[diag_el] = 1;
		}
		if (num_idempotent >= 0 && diag_el == idx) {
			idempotent_counter++;
			if (idempotent_counter > num_idempotent) {
				too_many++;
				return false;
			}
		}
	}
	if (counter + empty < num_diag) {
		too_few++;
		return false;
	}
	if ((num_idempotent >= 0) && idempotent_counter + empty < num_idempotent) {
		too_few++;
		return false;
	}
	good_continue++;
	return true;
}

bool
Partition::good_to_go(Cell Cells, int id, int el) {
	if (num_diag <= 0)
		return true;
	//std::cout << "debug start *****************************" << id << std::endl;
	//return true;
	if (base != Cells[id].get_base())
		return true;
	size_t row = (id - base) / domain_size;
	size_t column = (id - base) % domain_size;
	if (row != column)
		return true;

	int counter = count_diagonal(el);
	if (counter > num_diag)
		return false;
	if (counter + (domain_size - row) < num_diag)
		return false;

	return true;
}

bool
Partition::validate_model() {
	if (num_diag <= 0 && num_idempotent < 0)
		return true;

	if (count_diagonal(-1) == num_diag &&
			(num_idempotent < 0 || count_idempotent() == num_idempotent))
		return true;
	// std::cerr << "debug ******************************** failed validate_model() " << count_diagonal(-1) << std::endl;
	return false;
}
