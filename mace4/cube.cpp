
#include <ctime>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <unistd.h>
#include "syms.h"
#include "cube.h"


const std::string  Cube::steal_signal_file_path = "request_work.txt";
const std::string  Cube::steal_cube_file_path = "release_work.out";

Cube::~Cube() {
    // TODO Auto-generated destructor stub
}

bool
Cube::initialize_cube()
{
    // all_cubes is a vector of integers. Each 2 consective integers
    // are cell_id and cell_value
    initialized = false;
    if (all_cubes.empty()) 
        return initialized;
 

    std::istringstream config(all_cubes.front());
    int in_value;
    config >> in_value;
    ++num_input_cubes_processed;
    // if (num_input_cubes_processed % 1000 == 0)
    //    std::cout << "Debug Cube::initialize_cube " << num_input_cubes_processed << std::endl;
    while (config) {
        cube_cell_ids[max_pos] = in_value;
        config >> in_value;
        // std::cout << cube_cell_ids[max_pos] << "|" << in_value << " ";
        cell_values[cube_cell_ids[max_pos++]] = in_value;
        config >> in_value;
    }
    // std::cout << std::endl;
    all_cubes.erase(all_cubes.begin());  // first cube is to be processed now
    initialized = true;
    if (all_cubes.empty())
        std::cout << "Debug Cube::initialize last cube " << num_input_cubes_processed << std::endl;
    return initialized;
}

bool
Cube::reinitialize_cube()
{
    // return false when all cubes in the all_cubes store are done
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
    std::cout << "Debug Cube::read_config_multi " << std::endl;
    std::string line;
    while (std::getline(config, line))
    {
        std::string a_cube = line;
        // std::cout << "Debug cube line****" << line << "*****" << std::endl;
        all_cubes.push_back(a_cube);
    }
    return true;
}


Cube::Cube(size_t domain_size, Cell Cells, Cell Ordered_cells[], int Number_of_cells, int cubes_options):
        initialized(false), marked(false), order(domain_size), Cells(Cells), current_pos(0), max_pos(0), Ordered_cells(Ordered_cells),
        cut_off(5), mult_table_size(0), cubes_options(cubes_options), do_work_stealing(cubes_options & 1), 
        last_check_time(0), current_time(0), num_new_cubes_printed(0), num_input_cubes_processed(0)
{
    while (Ordered_cells[mult_table_size]->get_symbol() != "=" ) {
        mult_table_size++;
    }
    cut_off = mult_table_size * 9/10;

    for (size_t idx=0; idx<mult_table_size; idx++) {
        cell_ids.push_back(Ordered_cells[idx]->get_id());
    }
    int max_id = *std::max_element(std::begin(cell_ids), std::end(cell_ids));
    cell_values.resize(max_id+1, -1);
    cube_cell_ids.resize(max_id+1, -1);

    read_config_multi("cube.config");
    //    return;
    // at the very beginning, no cubes from config file means to start from the root
    initialized = true;
    if (!all_cubes.empty())
        initialize_cube();

    std::cout << "\ndebug constructor Cube*********************** cubes_options " << cubes_options << " max_pos = " << max_pos << std::endl;
    std::cout << "Debug ******************** cut_off: " << cut_off << std::endl;
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
Cube::value_assignment(int pos, int& value) 
{
   // for the pos-th value-assignment in the cube,
   // return the "term" as function return value, and the value of the assignment in "value"
   if (initialized && pos < max_pos) {
        value = cell_values[cube_cell_ids[pos]];
        return cube_cell_ids[pos];
   }
   else {
       value = -1;
       return -1;
   }
}

int
Cube::value(size_t depth, size_t id) {
    //if (initialized && current_pos < max_pos) {
    if (is_inside_input_cube()) {
        // std::cout << "Debug, Cube::value, incoming id " << id << " current_pos " << current_pos << " depth: " << depth << std::endl;
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
        // std::cout << "Debug Cube::value pos " << current_pos << " max position " << max_pos << " cell value: " << cell_values[id] << std::endl;
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
    std::cout << "debug work_stealing_requested: " << steal_signal_file_path.c_str() << signal_exists << " and " << steal_cube_file_path.c_str() << pending_cubes << std::endl;
    return signal_exists && !pending_cubes;
}

bool
Cube::move_on(size_t id, std::vector<std::vector<int>>& all_nodes) {
    if (!initialized)
        return false;
    if (current_time - last_check_time > min_check_interval) {
        last_check_time = current_time;
        // cut_off++;
        if (work_stealing_requested()) {
            // std::cout << "debug move_on, work_stealing_requested" << std::endl;
            if (!all_cubes.empty())
                return print_unprocessed_cubes(all_nodes, -1, 0, 0);
            size_t first_pos = 0;
	    // Find shortest cubes to print out
            while (all_nodes[first_pos][1] == all_nodes[first_pos][2]) {
                first_pos ++;
                if (first_pos >= all_nodes.size())
                    return false;
            }
            // std::cout << "debug move_on, first_pos " << first_pos << " " << all_nodes[first_pos][1]+1 << " " << all_nodes[first_pos][2] << std::endl;
            if (first_pos > cut_off)
                return false;
            if (print_unprocessed_cubes(all_nodes, all_nodes[first_pos][0], all_nodes[first_pos][1]+1, all_nodes[first_pos][2])) {
                all_nodes[first_pos][2] = all_nodes[first_pos][1];
                return true;
            }
        }
    }
    return false;
}

bool
Cube::print_unprocessed_cubes(const std::vector<std::vector<int>>& all_nodes, int root_id, size_t from, size_t to)
{
    bool ret_value = true;
    std::stringstream buffer;
    if (all_cubes.empty()) {
	std::cout << "print_unprocessed_cubes " << root_id << " " << from << " " << to << std::endl;
        for (size_t idx=from; idx<=to; idx++) {
            size_t pos = 0;
            while (all_nodes[pos][0] != root_id) {
                buffer << all_nodes[pos][0] << " " << Cells[all_nodes[pos][0]].get_value() << " ";
                pos++;
            }
            buffer << root_id << " " << idx << std::endl;
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
    // std::cout << "debug print_unprocessed_cubes: " << from << " " << to << "\n" << buffer.str() << std::endl;
    ofstream cube_file;
    cube_file.open (steal_cube_file_path);
    cube_file << buffer.str();
    cube_file.close();

    if (remove(steal_signal_file_path.c_str()) != 0) {
        std::cout << "@@@@@@@@@@Cube::print_unprocessed_cubes@@@@@@@@@@@@@@@@@@failed to delete signal file " << steal_signal_file_path.c_str() << std::endl;
    }
    return ret_value;
}


bool
Cube::count_appearance(bool& counted, size_t base_id, size_t id, std::vector<size_t>& appear) 
{
    // count the number of times a domain element appears as a value in the cube, up to
    // but not including the cell with id
    if (!counted) {
        for (size_t ptr=0; ptr<id; ++ptr)
            appear[Cells[base_id + ptr].get_value()] += 1;
    }
    return true;
}


bool
Cube::has_value(size_t start_id, size_t end_id, int val)
{
    for (size_t ptr=0; ptr < end_id; ++ptr) {
        if (Cells[ptr].get_value() == val)
            return true;
    }
    return false;
}

int
Cube::sum_val(int& sum, bool& counted, size_t base_id, size_t id, std::vector<size_t>& appear)
{
    if (sum > -1)
        return sum;
    count_appearance(counted, base_id, id, appear);
    sum = accumulate(appear.begin()+1, appear.end(), 0);
    return sum;
}


void
Cube::print_new_cube(ofstream& cubes_file, int cube_length, const std::vector<std::vector<int>>&  all_nodes, const std::string& cg) 
{
    bool same = true;
    if (last_printed.size() < cube_length)
        last_printed.insert(last_printed.end(), cube_length, -1);
    for (int idx = 0; idx < cube_length; ++idx) {
        if (last_printed[idx] != Cells[all_nodes[idx][0]].get_value()) {
            last_printed[idx] = Cells[all_nodes[idx][0]].get_value();
            same = false;
        }
    }
    if (same) {
        return;
    }
    /* debug print
      for (int idx = 0; idx < print_cubes; ++idx)
          std::cout << " " << splitter.cell_ids[idx] << "|" << Cells[splitter.cell_ids[idx]].get_symbol();
      std::cout << std::endl;
      */
    // std::cout << "cube";
    const char* spacer = "";
    for (int idx = 0; idx < cube_length; ++idx) {
        cubes_file << spacer << all_nodes[idx][0] << " " << last_printed[idx];
        if (idx <= 0)  spacer = " ";
    }
    cubes_file << std::endl;
    num_new_cubes_printed++;
}

bool
Cube::is_end_of_circle(int id)
{
    // returns true if this is the last cell in the search of a concentric circle
    int id_pos = find_pos_in_search(id);
    if (id_pos > 0 && id_pos+1 < cell_ids.size()) {
        int next_id = cell_ids[id_pos+1];
        int next_row = Cells[next_id].get_index(0);
        int next_col = Cells[next_id].get_index(1);
        if (next_row == next_col) {
            return true;
        }
    }
    return false;
}

size_t
Cube::find_pos_in_search(int id)
{
    // TODO optimize - no need to re-traverse everytime
    for (size_t pos=0; pos < cell_ids.size(); ++pos) {
        if (cell_ids[pos] == id)
            return pos;
    }
    return -1;
}


int Cube::moved_to_cell(int orig_id, size_t low_el, size_t high_el)
{
    size_t arity = Cells[orig_id].get_arity();
    size_t f_idx = Cells[orig_id].get_index(0);
    size_t f_offset = 0;
    if (f_idx == low_el)
        f_offset = high_el - low_el;
    else if (f_idx == high_el)
        f_offset = low_el - high_el;
    if (arity == 1)
        return orig_id + f_offset;

    size_t s_idx = Cells[orig_id].get_index(1);
    size_t s_offset = 0;
    if (s_idx == low_el)
        s_offset =  high_el - low_el;
    else if (s_idx == high_el)
        s_offset = low_el - high_el;
    if (arity == 2) 
        return orig_id + f_offset * order + s_offset;
    else    
        return -1; 
}


int
Cube::check_binop(size_t row, size_t col,
                  const std::vector<size_t>& row_idx, const std::vector<size_t>& col_idx, 
                  const std::vector<size_t>& binop)
{
    // check whether binop is made lex smaller when permuted by the permutation
    // of rows and columns (represented by pi1 and pi2).

    size_t col_end = order;
    // Check each cell, row by row and column by column starting with row zero
    for (size_t ridx=0; ridx <= row; ++ridx) {
        if (row == ridx)
            col_end = col + 1;
        for (size_t cidx = 0; cidx < col_end; ++cidx) {
            int diff = row_idx[col_idx[Cells[binop[row_idx[col_idx[ridx]]] + row_idx[col_idx[cidx]]].get_value()]] - 
                       Cells[binop[ridx] + cidx].get_value();
            if (diff != 0)
                return diff;
        }
    }
    return 0;
}

