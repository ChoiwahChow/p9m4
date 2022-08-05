
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include "syms.h"
#include "cube.h"


const std::string  Cube::steal_signal_file_path = "request_work.txt";
const std::string  Cube::steal_cube_file_path = "release_work.out";

Cube::~Cube() {
	// TODO Auto-generated destructor stub
}

Cube::Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells, int cubes_options): initialized(false),
		order(domain_size), Cells(Cells), current_pos(0), branch_depth(Number_of_cells+1), max_pos(0), branch_root_id(-1),
		cubes_options(cubes_options), do_work_stealing(cubes_options & 1) {
	cell_values.insert(cell_values.end(), Number_of_cells, -1);
	ifstream config("cube.config");
	for (size_t idx=0; idx<Number_of_cells; idx++)
		cell_ids.push_back(Ordered_cells[idx]->get_id());
	if (!config.is_open()) {
		return;
	}
	int cell_value;
	config >> cell_value;
	std::cout << "Debug Cube::Cube ";
	while (!config.eof()) {
		std::cout << cell_ids[max_pos] << "|" << cell_value << " ";
		cell_values[cell_ids[max_pos++]] = cell_value;
		config >> cell_value;
	}
	config.close();
	initialized = true;

	std::cout << "\ndebug Cube*********************** max_depth = " << max_pos-1 << std::endl;
    // /* debug
    for (int idx = 0; idx < Number_of_cells && idx < 65; ++idx)
	  std::cout <<  idx << "|" << Ordered_cells[idx]->get_symbol() << "|" << Ordered_cells[idx]->get_id() << "  ";
    std::cout << "Debug order_cells ********************" << std::endl;
}

size_t
Cube::real_depth(size_t depth, size_t id) {
	for (size_t idx=depth; idx<cell_ids.size(); ++idx)
		if (cell_ids[idx]==id)
			return idx;
	return depth;
}

int
Cube::value(size_t depth, size_t id) {
	if (initialized && current_pos < max_pos) {
		std::cout << "Debug, Cube::value, incoming id " << id << " current_pos " << current_pos << std::endl;
		while (id != cell_ids[current_pos] && current_pos < max_pos) {
			if ((Cells[cell_ids[current_pos]].get_value() != cell_values[cell_ids[current_pos]]) && (cell_values[cell_ids[current_pos]] != -1)) {
				std::cout << "Debug, Cube::value, Mis-matched cell id =" << cell_ids[current_pos] << ", cell value = " << cell_values[cell_ids[current_pos]]
						  << "  vs  " << Cells[cell_ids[current_pos]].get_value() << std::endl;
				current_pos++;
				return order+1;
			}
			// std::cout << "Debug, Cube::value, matched, cell id = "  << cell_ids[current_pos] << "  current pos " << current_pos << std::endl;
			current_pos++;
		}
		if (current_pos >= max_pos) {
			return -1;
		}
		if (id == cell_ids[current_pos]) {
			current_pos++;
		}
		std::cout << "Debug value pos" << current_pos << " max position " << max_pos << std::endl;
		return cell_values[id];
	}
	return -1;
}

inline bool
Cube::work_stealing_requested() {
	struct stat buffer;
	bool signal_exists = stat (steal_signal_file_path.c_str(), &buffer) == 0;
	bool pending_cubes = stat (steal_cube_file_path.c_str(), &buffer) == 0;
	time_t now = time(0);
	char* date_time = ctime(&now);
	std::cout << "debug Current working directory: " << get_current_dir_name() << " " << date_time << std::endl;
	std::cout << "debug work_stealing_requested:" << steal_signal_file_path.c_str() << signal_exists << " and" << steal_cube_file_path.c_str() << pending_cubes << std::endl;
	return signal_exists && !pending_cubes;
}

bool
Cube::move_on(size_t id, int val, int last, int level_1, int level_2, int level_3) {
	/*
	 * level_1 is the parent cell's id, level_2 is the grand-parent cell's id.  If the root of the search sub tree
	 * is a parent or a grand-parent of the current cell, then check if working stealing is asked for.
	 */
	if (branch_root_id != -1 && do_work_stealing) {
		if (id == branch_root_id) {
			//std::cout << "debug move_on, back to top root " << id << std::endl;
			if (val == last) {  // move root to the next level
				branch_root_id = -1;
				return false;
			}
			if (work_stealing_requested()) {
				//std::cout << "debug move_on, work_stealing_requested, branch_root_id reset" << std::endl;
				print_unprocessed_cubes(id, val+1, last);
				branch_root_id = -1;
				return true;
			}
		}
		else if (level_1 == branch_root_id || level_2 == branch_root_id || level_3 == branch_root_id) {
			if (val == last)
				return false;
			if (work_stealing_requested()) {
				//std::cout << "debug move_on, work_stealing_requested, parent/grandparent,  " << id << std::endl;
				print_unprocessed_cubes(id, val+1, last);
				return true;
			}
		}
	}
	return false;
}

void
Cube::print_unprocessed_cubes(int root_id, size_t from, size_t to)
{
	std::stringstream buffer;
	for (size_t idx=from; idx<=to; idx++) {
		size_t pos = 0;
		while (cell_ids[pos] != root_id) {
			buffer << Cells[cell_ids[pos]].get_value() << " ";
			pos++;
		}
		buffer << idx << std::endl;
	}
	buffer << "End" << std::endl;
	std::cout << "debug print_unprocessed_cubes: " << from << " " << to << "\n" << buffer.str() << std::endl;
	ofstream cube_file;
	cube_file.open (steal_cube_file_path);
	cube_file << buffer.str();
	cube_file.close();

	if (remove(steal_signal_file_path.c_str()) != 0) {
		std::cout << "failed to delete signal file" << std::endl;
	}
}

void
Cube::mark_root(size_t id) {
	if (initialized && branch_root_id == -1) {
		branch_root_id = id;
		std::cout << "debug mark_root: " << id << std::endl;
	}
}

void
Cube::print_new_cube(int cube_length) {
	bool same = true;
	if (last_printed.size() < cube_length)
		last_printed.insert(last_printed.end(), cube_length, -1);
  	for (int idx = 0; idx < cube_length; ++idx) {
  	  if (last_printed[idx] != Cells[cell_ids[idx]].get_value()) {
  		  last_printed[idx] = Cells[cell_ids[idx]].get_value();
  		  same = false;
  	  }
  	}
  	if (same)
  		return;
	/* debug print
  	std::cout << "model: depth " << depth << " cell";
  	for (int idx = 0; idx < print_cubes; ++idx)
      std::cout << " " << splitter.cell_ids[idx] << "|" << Cells[splitter.cell_ids[idx]].get_symbol();
  	std::cout << std::endl;
  	*/
  	std::cout << "cube";
  	for (int idx = 0; idx < cube_length; ++idx)
  	  std::cout << " " << last_printed[idx];
  	std::cout << std::endl;
}



