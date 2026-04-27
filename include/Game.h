#ifndef GAME_H
#define GAME_H
#include <vector>
#include <string>
#include <omp.h>
#include "World.cuh"
#include <thread>

// old run time without openmp for 100x100 random 6 run 10: 178309 
// new run: 1039.34

class Game
{
    private:
        int m_height {};
        int m_width {};
        std::string m_alive_cell {"\033[1m\033[32m\u2593\u2593\033[0m\033"};
        std::string m_dead_cell {"\033[1m\033[90m\u2591\u2591\033[0m"};
        // save 3 states
        std::vector<std::vector<bool>> m_previous2_state;
        std::vector<std::vector<bool>> m_previous_state;
        std::vector<std::vector<bool>> m_current_state;

        // 3 states as arrays
        bool* d_current;
        bool* d_previous;
        bool* d_previous2;

        // pinned host memory
        bool* h_current;
        bool* h_previous;
        bool* h_previous2;

        // CUDA Stream
        cudaStream_t m_stream;

        std::string m_begin_output;


        bool check_x(int x) { return (x>=0 && x < m_height); }
        bool check_y(int y) { return (y>=0 && y < m_width); }

        int get_x_m1(int x) { return (x-1) < 0 ? (m_height -1) : (x-1); }
        int get_x_p1(int x) { return (x+1) >= m_height ? 0 : (x+1); }

        int get_y_m1(int y) { return (y-1) < 0 ? (m_width-1) : (y-1); }
        int get_y_p1(int y) { return (y+1) >= m_width ? 0 : (y+1); }



    public:
        Game(int height, int width, std::vector<std::vector<bool>> previous2_state,
            std::vector<std::vector<bool>> previous_state,
            std::vector<std::vector<bool>> current_state);

        Game(std::string file_name);

        ~Game();

        void evolve();

        void evolveCUDA(int num_iter, bool printFlag, int delay_ms);
        void updateCUDA(bool all3States);

        void evolveLinear();



        bool cell_future(bool cell_state, int x, int y);

        bool is_stable();

        void print();

        void load(std::string file_name);
        void save(std::string file_name);
        
        // getters
        int get_width() { return m_width; }
        int get_height() { return m_height; }

        // todo getters for states
        // todo setters for states

        // setters
        void set_width(int width) { m_width = width; }
        void set_height(int height) { m_height = height; }



        // Set cell state 2d
        void insert_cell(int x, int y, bool status) 
        {
            if(check_x(x) && check_y(y))
                m_current_state[x][y] = status;
        }

        // Set cell state 1d
        void insert_cell(int cell_nr, bool status)
        {
            int x = static_cast<int>(cell_nr / m_width);
            int y = cell_nr % m_width;
            m_current_state[x][y] = status;
        }

        // Get cell state 2d
        bool get_cell(int x, int y)
        {
            if(check_x(x) && check_y(y))
                return m_current_state[x][y];
        }

        // Get cell state 1d
        bool get_cell(int cell_nr)
        {
            int x = static_cast<int>(cell_nr / m_width);
            int y = cell_nr % m_width;
            return m_current_state[x][y];

        }

        // add glider pattern
        void add_glider(int x, int y);

        // add toad pattern
        void add_toad(int x, int y);

        // add beacon pattern
        void add_beacon(int x, int y);

        // add Methuselah pattern
        void add_methuselah(int x, int y);

        // add n random patterns
        void add_n_random(int n);

};
#endif