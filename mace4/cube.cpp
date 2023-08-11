
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

bool
Cube::read_config(const char* config_file_path) {
	ifstream config(config_file_path);
	if (!config.is_open()) {
		return false;
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
	return true;
}


bool
Cube::initialize_cube()
{
	initialized = false;
	if (all_cubes.empty())
		return initialized;

	std::istringstream config(all_cubes.front());
	int cell_value;
	config >> cell_value;
	std::cout << "Debug Cube::Cube ";
	while (config) {
		std::cout << cell_ids[max_pos] << "|" << cell_value << " ";
		cell_values[cell_ids[max_pos++]] = cell_value;
		config >> cell_value;
	}
	all_cubes.erase(all_cubes.begin());  // first cube is to be processed now
	initialized = true;
	return initialized;
}

bool
Cube::reinitialize_cube()
{
	max_pos = 0;
	current_pos = 0;
	last_printed.clear();
	return initialize_cube();
}

bool
Cube::read_config_multi(const char* config_file_path) {
	ifstream config(config_file_path);
	if (!config.is_open()) {
		return false;
	}
	std::cout << "Debug Cube::Cube ";
	std::string line;
	while (std::getline(config, line))
	{
		std::string a_cube = line;
		std::cout << "Debug cube line****" << line << "*****" << std::endl;
		all_cubes.push_back(a_cube);
	}
	return true;
}


Cube::Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells, int cubes_options): initialized(false),
		marked(false), order(domain_size), Cells(Cells), current_pos(0), max_pos(0), cut_off(5), mult_table_size(0),
		cubes_options(cubes_options), do_work_stealing(cubes_options & 1), last_check_time(0), current_time(0) {
	while (Ordered_cells[mult_table_size]->get_symbol() != "=" ) {
		mult_table_size++;
	}
	cut_off = mult_table_size * 9/10;

	for (size_t idx=0; idx<mult_table_size; idx++) {
		cell_ids.push_back(Ordered_cells[idx]->get_id());
	}
	int max_id = *std::max_element(std::begin(cell_ids), std::end(cell_ids));
	cell_values.resize(max_id+1, -1);
	real_depths.resize(max_id+1, 0);
    for (size_t idx = 0; idx < mult_table_size; ++idx)
    	real_depths[cell_ids[idx]] = idx;

	if (!read_config_multi("cube.config"))
		return;
	initialized = initialize_cube();
	if (!initialized)
		return;

	std::cout << "\ndebug Cube*********************** cubes_options " << cubes_options << " max_depth = " << max_pos-1 << std::endl;
    // /* debug
    for (size_t idx = 0; idx < mult_table_size && idx < 65; ++idx) {
	  std::cout <<  idx << "|" << Ordered_cells[idx]->get_symbol() << "|" << Ordered_cells[idx]->get_id() << "  ";
    }
	std::cout << "Debug order_cells ********************" << std::endl;
    for (size_t idx = 0; idx < mult_table_size && idx < 65; ++idx) {
	  std::cout <<  idx << "|" << real_depths[idx] << "  ";
    }
	std::cout << "Debug real_depths end ******************** mult_table_size: " << mult_table_size << " Number_of_cells:" << Number_of_cells << std::endl;
	std::cout << "Debug ******************** cut_0ff: " << cut_off << std::endl;
}

size_t
Cube::real_depth(size_t depth, size_t id) {
	return real_depths[id];
}

int
Cube::num_cells_filled(Cell Cells)
{
	size_t count = 0;
	for (size_t idx=0; idx<cell_ids.size(); idx++){
		if (Cells[cell_ids[idx]].has_value())
			count++;
	}
	// std::cout << "Debug Num cells filled " << count << std::endl;
	return count;
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
	if (!signal_exists)
		return false;
	bool pending_cubes = stat (steal_cube_file_path.c_str(), &buffer) == 0;
	// time_t now = time(0);
	// char* date_time = ctime(&now);
	// std::cout << "debug Current working directory: " << get_current_dir_name() << " " << date_time << std::endl;
	std::cout << "debug work_stealing_requested:" << steal_signal_file_path.c_str() << signal_exists << " and" << steal_cube_file_path.c_str() << pending_cubes << std::endl;
	return signal_exists && !pending_cubes;
}

bool
Cube::move_on(size_t id, std::vector<std::vector<int>>& all_nodes) {
	// std::cout << "debug @@@@@@@@@@@@ " << real_depths.size() << " " << real_depths[id] << " " << cut_off << std::endl;
	/*
	if (marked && all_cubes.empty() && real_depths[id] < early_cut_off) {
		for (size_t idx=first+1; idx<=last; idx++ ) {
			size_t jdx = 0;
			std::string cube(std::to_string(Cells[cell_ids[jdx++]].get_value()));
			while (cell_ids[jdx] != id) {
				cube.append(" ");
				cube.append(std::to_string(Cells[cell_ids[jdx++]].get_value()));
			}
			cube.append(" ");
			cube.append(std::to_string(idx));
			all_cubes.push_back(cube);
		}
		return true;
	}
	*/
	if (!initialized)
		return false;
	if (all_cubes.empty() && real_depths[id] > cut_off)
		return false;
	if (current_time - last_check_time > min_check_interval) {
		last_check_time = current_time;
		// cut_off++;
		if (work_stealing_requested()) {
			//std::cout << "debug move_on, work_stealing_requested" << std::endl;
			if (!all_cubes.empty())
				return print_unprocessed_cubes(-1, 0, 0);
			size_t first_pos = 0;
			while (all_nodes[first_pos][1] == all_nodes[first_pos][2]) {
				first_pos ++;
				if (first_pos >= all_nodes.size())
					return false;
			}
			if (real_depths[all_nodes[first_pos][0]] > cut_off)
				return false;
			if (print_unprocessed_cubes(all_nodes[first_pos][0], all_nodes[first_pos][1]+1, all_nodes[first_pos][2])) {
				all_nodes[first_pos][2] = all_nodes[first_pos][1];
				return true;
			}
		}
	}
	return false;
}

bool
Cube::print_unprocessed_cubes(int root_id, size_t from, size_t to)
{
	bool ret_value = true;
	std::stringstream buffer;
	if (all_cubes.empty()) {
		for (size_t idx=from; idx<=to; idx++) {
			size_t pos = 0;
			while (cell_ids[pos] != root_id) {
				buffer << Cells[cell_ids[pos]].get_value() << " ";
				pos++;
			}
			buffer << idx << std::endl;
		}
	}
	else {
		ret_value = false;
		size_t last = all_cubes.size() / 2;
		if (last == 0) {
			buffer << all_cubes[0] << std::endl;
			all_cubes.erase(all_cubes.begin());
		}
		else {
			for(std::vector<std::string>::iterator it = std::begin(all_cubes); it != std::begin(all_cubes)+last; ++it) {
				buffer << *it << std::endl;
			}
			all_cubes.erase(all_cubes.begin(), all_cubes.begin()+last);
		}
	}
	buffer << "End" << std::endl;
	std::cout << "debug print_unprocessed_cubes: " << from << " " << to << "\n" << buffer.str() << std::endl;
	ofstream cube_file;
	cube_file.open (steal_cube_file_path);
	cube_file << buffer.str();
	cube_file.close();

	if (remove(steal_signal_file_path.c_str()) != 0) {
		std::cout << "@@@@@@@@@@Cube::print_unprocessed_cubes@@@@@@@@@@@@@@@@@@failed to delete signal file " << steal_signal_file_path.c_str() << std::endl;
	}
	return ret_value;
}

void
Cube::print_new_cube(int cube_length, int num_cells_filled, const std::string& cg) {
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
    // std::cout << " " << num_cells_filled;
    for (int idx = 0; idx < cube_length; ++idx)
        std::cout << " " << last_printed[idx];
    if (!cg.empty())
        std::cout << " %c%" << cg;
    std::cout << std::endl;
}



