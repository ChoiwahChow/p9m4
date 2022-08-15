
#ifndef MACE4_CUBE_H
#define MACE4_CUBE_H

#include <sys/stat.h>
#include <string>
#include <vector>
#include "cell.h"

class Cube {
public:
	static const std::string  steal_signal_file_path;
	static const std::string  steal_cube_file_path;
	std::vector<size_t> cell_ids;
	int                 branch_root_id;
	int                 cubes_options;
private:
	std::vector<int>  cell_values;
	size_t            order;
	// size_t            branch_depth;
	Cell              Cells;
	bool              initialized;
	size_t            current_pos;  // current position of the search in Ordered_cells
	size_t            max_pos;      // max value of current position
	std::vector<int>  last_printed;
	bool              do_work_stealing;
	std::vector<std::string> all_cubes;

private:
	bool print_unprocessed_cubes(int root_id, size_t from, size_t to);
	inline bool work_stealing_requested ();
	bool read_config(const char* config_file_path);
	bool read_config_multi(const char* config_file_path);
	bool initialize_cube();

public:
	Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells, int cube_options);
	virtual ~Cube();

	bool reinitialize_cube();
	int  value(size_t depth, size_t id);
	void print_new_cube(int cube_length);
	void mark_root(size_t id);
	bool move_on(size_t id, int val, int last, int level_1, int level_2, int level_3);
	size_t real_depth(size_t depth, size_t id);
	// void print_ordered_cells(int number_of_cells) const;
};


#endif
