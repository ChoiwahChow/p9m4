
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
	static const int min_check_interval = 2;
	std::vector<size_t> cell_ids;
	int                 cubes_options;
private:
	std::vector<int>  cell_values;
	size_t            order;
	std::vector<size_t>  real_depths;
	Cell              Cells;
	bool              initialized;
	bool			  marked;       // root id marked for the process
	size_t            current_pos;  // current position of the search in Ordered_cells
	size_t            max_pos;      // max value of current position
	size_t            cut_off;      // must be less than cut_off from the start cube position to release cubes for work stealing
	size_t			  early_cut_off;
	size_t			  top_cut_off;
	size_t            mult_table_size;    // total size of multiplication tables
	std::vector<int>  last_printed;
	bool              do_work_stealing;
	std::vector<std::string> all_cubes;
	size_t            last_check_time;
	size_t			  current_time;

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
	void set_time(size_t seconds) {current_time = seconds;}
	int  value(size_t depth, size_t id);
	void print_new_cube(int cube_length);
	// size_t mark_root(size_t id, size_t from_index, size_t last);
	bool move_on(size_t id, std::vector<std::vector<int>>& all_nodes);
	size_t real_depth(size_t depth, size_t id);
	// void print_ordered_cells(int number_of_cells) const;
};


#endif
