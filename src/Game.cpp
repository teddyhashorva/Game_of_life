#include "../include/Game.h"

#include <iostream>
#include <fstream>

#include <random>
#include <string>

// Constructor implementation
Game::Game(int height, int width, std::vector<std::vector<bool>> previous2_state,
            std::vector<std::vector<bool>> previous_state,
            std::vector<std::vector<bool>> current_state)
{

    m_height = height;
    m_width = width;
    m_previous2_state = previous2_state;
    m_previous_state = previous_state;
    m_current_state = current_state;

    m_begin_output = "”\033[2J\033[H";

    size_t size = m_height * m_width * sizeof(bool);

    // Unified Memory Allocation version
    // checkCuda(cudaMallocManaged(&d_current, size));
    // checkCuda(cudaMallocManaged(&d_previous, size));
    // checkCuda(cudaMallocManaged(&d_previous2, size));

    
    // Manual Memory Allocation version
    checkCuda(cudaMallocHost(&h_current, size));
    checkCuda(cudaMallocHost(&h_previous, size));
    checkCuda(cudaMallocHost(&h_previous2, size));

    checkCuda(cudaMalloc(&d_current, size));
    checkCuda(cudaMalloc(&d_previous, size));
    checkCuda(cudaMalloc(&d_previous2, size));



    for(int i = 0; i < m_height; i++)
    {
        for(int j =0; j<m_width;j++)
        {
            int idx = i*m_width + j;
            h_current[idx] = m_current_state[i][j];
            h_previous[idx] = m_previous_state[i][j];
            h_previous2[idx] = m_previous2_state[i][j];
        }
    }

    checkCuda(cudaStreamCreate(&m_stream));

    checkCuda(cudaMemcpyAsync(d_current, h_current, size, cudaMemcpyHostToDevice, m_stream));
    checkCuda(cudaMemcpyAsync(d_previous, h_previous, size, cudaMemcpyHostToDevice, m_stream));
    checkCuda(cudaMemcpyAsync(d_previous2, h_previous2, size, cudaMemcpyHostToDevice, m_stream));   

    checkCuda(cudaStreamSynchronize(m_stream));
}

Game::Game(std::string file_name)
{
    std::ifstream input_file (file_name);

    if(!input_file.is_open())
        throw std::runtime_error("Couldn't open file: " + file_name);

    int height, width;
    input_file >> height;
    input_file >> width;

    std::vector<std::vector<bool>> state(height, std::vector<bool>(width));
    std::vector<std::vector<bool>> prev_state(height, std::vector<bool>(width));
    std::vector<std::vector<bool>> prev2_state(height, std::vector<bool>(width));
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            int cellValue;
            input_file >> cellValue;
            state[i][j] = (cellValue == 1);
            prev_state[i][j] = false;
            prev2_state[i][j] = false;
        }
    }
    m_height = height;
    m_width = width;
    m_current_state = state;
    m_previous2_state = prev2_state;
    m_previous_state = prev_state;

    m_begin_output = "”\033[2J\033[H";

    size_t size = m_height * m_width * sizeof(bool);

    // Unified Memory Allocation version
    // cudaMallocManaged(&d_current, size);
    // cudaMallocManaged(&d_previous, size);
    // cudaMallocManaged(&d_previous2, size);


    // Manual Memory allocation version
    checkCuda(cudaMallocHost(&h_current, size));
    checkCuda(cudaMallocHost(&h_previous, size));
    checkCuda(cudaMallocHost(&h_previous2, size));

    checkCuda(cudaMalloc(&d_current, size));
    checkCuda(cudaMalloc(&d_previous, size));
    checkCuda(cudaMalloc(&d_previous2, size));

    for(int i = 0; i < m_height; i++)
    {
        for(int j =0; j<m_width;j++)
        {
            int idx = i*m_width + j;
            h_current[idx] = m_current_state[i][j];
            h_previous[idx] = m_previous_state[i][j];
            h_previous2[idx] = m_previous2_state[i][j];
        }
    }

    checkCuda(cudaStreamCreate(&m_stream));

    checkCuda(cudaMemcpyAsync(d_current, h_current, size, cudaMemcpyHostToDevice, m_stream));
    checkCuda(cudaMemcpyAsync(d_previous, h_previous, size, cudaMemcpyHostToDevice, m_stream));
    checkCuda(cudaMemcpyAsync(d_previous2, h_previous2, size, cudaMemcpyHostToDevice, m_stream));

    checkCuda(cudaStreamSynchronize(m_stream));
    
    

}

bool Game::cell_future(bool cell_state, int x, int y)
{
    // rule1: live with fewer than two live neighbours dead
    // rule2: live with 2 or 3 live neighbours -> alive
    // rule3: live with > 3 -> dead
    // rule4: dead with 3 live -> alive

    // Getting Neighbours coordinates
    int x_m1 = get_x_m1(x);
    int x_p1 = get_x_p1(x);

    int y_m1 = get_y_m1(y);
    int y_p1 = get_y_p1(y);

    int sum = 0;

    sum += static_cast<int>(m_current_state[x_m1][y_m1]);
    sum += static_cast<int>(m_current_state[x_m1][y]);
    sum += static_cast<int>(m_current_state[x_m1][y_p1]);
    sum += static_cast<int>(m_current_state[x][y_m1]);
    sum += static_cast<int>(m_current_state[x][y_p1]);
    sum += static_cast<int>(m_current_state[x_p1][y_m1]);
    sum += static_cast<int>(m_current_state[x_p1][y]);
    sum += static_cast<int>(m_current_state[x_p1][y_p1]);


    // applying rules
    if(cell_state)
        if(sum >= 2 && sum <= 3)
            return true;
        else
            return false;
    else
        if(sum == 3)
            return true;
        return false;
}

void Game::evolve()
{
    // Here we can use openMP  {Alte Version}

    const int available_procs = omp_get_num_procs();
    const int nThreads = std::max(1, available_procs - 2);
    omp_set_num_threads(nThreads);

    #pragma omp parallel for 
    for(int i=0; i < m_height; i++)
    {
        for(int j=0; j < m_width; j++)
        {
            m_previous2_state[i][j] = cell_future(m_current_state[i][j], i, j);
        }
    }

    // Swapping current with prev2
    std::swap(m_current_state, m_previous2_state);

    // Swapping prev with prev2
    std::swap(m_previous_state, m_previous2_state);

}

void Game::evolveCUDA(int num_iter, bool printFlag, int delay_ms)
{
    size_t size = m_height * m_width * sizeof(bool);

    // Unified Memory allocation version of CUDA evolve
    // evolveWorldCUDA(d_current, d_previous, d_previous2, m_width, m_height, m_stream);

    // std::swap(d_current, d_previous2);
    // std::swap(d_previous, d_previous2);

    // // Copy to m_current_state
    // for(int i=0; i<m_height;i++)
    //     for(int j=0; j<m_width;j++)
    //         m_current_state[i][j] = d_current[i*m_width +j];


    

    // Copy from host to device
    checkCuda(cudaMemcpyAsync(d_current, h_current, size, cudaMemcpyHostToDevice, m_stream));
    checkCuda(cudaMemcpyAsync(d_previous, h_previous, size, cudaMemcpyHostToDevice, m_stream));
    checkCuda(cudaMemcpyAsync(d_previous2, h_previous2, size, cudaMemcpyHostToDevice, m_stream));

    for(int i = 0; i < num_iter; ++i)
    {
        evolveWorldCUDA(d_current, d_previous, d_previous2, m_width, m_height, m_stream);

        // Logic if printFlag is true
        if(printFlag)
        {
            std::cout<<m_begin_output;
            checkCuda(cudaMemcpyAsync(h_current, d_current, size, cudaMemcpyDeviceToHost, m_stream));
            checkCuda(cudaStreamSynchronize(m_stream));

            updateCUDA(false);
            print();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

        std::swap(d_current, d_previous2);
        std::swap(d_previous, d_previous2);
    }

    // std::swap(d_current, d_previous2);
    // std::swap(d_previous, d_previous2);

    // Copy from device to host
    checkCuda(cudaMemcpyAsync(h_current, d_current, size, cudaMemcpyDeviceToHost, m_stream));

    checkCuda(cudaMemcpyAsync(h_previous, d_previous, size, cudaMemcpyDeviceToHost, m_stream));
    checkCuda(cudaMemcpyAsync(h_previous2, d_previous2, size, cudaMemcpyDeviceToHost, m_stream));

    
    checkCuda(cudaStreamSynchronize(m_stream));
}

void Game::updateCUDA(bool all3States){
    
    for(int i=0; i<m_height;i++)
         for(int j=0; j<m_width;j++)
         {
            m_current_state[i][j] = h_current[i*m_width +j];

            if(all3States)
            {
                m_previous_state[i][j] = h_previous[i*m_width + j];
                m_previous2_state[i][j] = h_previous2[i*m_width + j];
            }
         }
}

void Game::evolveLinear()
{
    for(int i=0; i < m_height; i++)
    {
        for(int j=0; j < m_width; j++)
        {
            m_previous2_state[i][j] = cell_future(m_current_state[i][j], i, j);
        }
    }

    // Swapping current with prev2
    std::swap(m_current_state, m_previous2_state);

    // Swapping prev with prev2
    std::swap(m_previous_state, m_previous2_state);

}

void Game::print()
{
    // Since we want the layout to correspond to the game, we cannot use opoenMP here, otherwise we would either make the execution linear + overhead
    // or simply the layout would not correspond to the gamestate anymore
    for(int i=0; i < m_height; i++)
    {
        for(int j=0; j < m_width; j++)
        {
            if(m_current_state[i][j])
                std::cout << m_alive_cell;
            else
                std::cout << m_dead_cell;
        }

        std::cout << std::endl;
    }
}

bool Game::is_stable()
{
    // Here we can use openMP again

    const int available_procs = omp_get_num_procs();
    const int nThreads = std::max(1, available_procs - 2);
    omp_set_num_threads(nThreads);

    bool stable = true;

    #pragma omp parallel for shared(stable)
    for(int i=0; i < m_height; i++)
    {
        if(!stable) continue;

        for(int j=0; j<m_width; j++)
        {
            if(!stable) break;

            if( (m_current_state[i][j] != m_previous_state[i][j]) && (m_current_state[i][j] != m_previous2_state[i][j]) ) {
                stable = false;
            }
                
        }
    }

    return stable;
}

// Patterns

// Glider Pattern
void Game::add_glider(int x, int y)
{
    if(check_x(x) && check_y(y))
    {
        m_current_state[x][y] = true;

        // Getting other cells of the pattern
        int x_p1 = get_x_p1(x);
        int y_p1 = get_y_p1(y);

        int x_p2 = get_x_p1(x_p1);
        int y_m1 = get_y_m1(y);

        // Setting other cells
        m_current_state[x_p1][y_p1] = true;
        m_current_state[x_p2][y_m1] = true;
        m_current_state[x_p2][y] = true;
        m_current_state[x_p2][y_p1] = true;

        size_t size = m_height * m_width * sizeof(bool);
    
        // unified memory allocation version
        // for(int i = 0; i < m_height; i++)
        // {
        //     for(int j =0; j<m_width;j++)
        //     {
        //         int idx = i*m_width + j;
        //         d_current[idx] = m_current_state[i][j];
        //         d_previous[idx] = m_previous_state[i][j];
        //         d_previous2[idx] = m_previous2_state[i][j];
        //     }
        // }

        for(int i = 0; i < m_height; i++)
        {
            for(int j =0; j<m_width;j++)
            {
                int idx = i*m_width + j;
                h_current[idx] = m_current_state[i][j];
                h_previous[idx] = m_previous_state[i][j];
                h_previous2[idx] = m_previous2_state[i][j];
            }
        }
    }
}

// Toad pattern
void Game::add_toad(int x, int y)
{
    if(check_x(x) && check_y(y))
    {

        m_current_state[x][y] = true;

        // Getting other cells of the pattern
        int y_p1 = get_y_p1(y);
        int y_p2 = get_y_p1(y_p1);

        int y_m1 = get_y_m1(y);
        int x_p1 = get_x_p1(x);

        // Setting other cells
        m_current_state[x][y_p1] = true;
        m_current_state[x][y_p2] = true;
        m_current_state[x_p1][y_m1] = true;
        m_current_state[x_p1][y] = true;
        m_current_state[x_p1][y_p1] = true;
    }

    for(int i = 0; i < m_height; i++)
    {
        for(int j =0; j<m_width;j++)
        {
            int idx = i*m_width + j;
            h_current[idx] = m_current_state[i][j];
            h_previous[idx] = m_previous_state[i][j];
            h_previous2[idx] = m_previous2_state[i][j];
        }
    }


}

// Beacon pattern
void Game::add_beacon(int x, int y)
{
    if(check_x(x) && check_y(y))
    {
        m_current_state[x][y] = true;

        // Getting other cells of the pattern
        int y_p1 = get_y_p1(y);
        int x_p1 = get_x_p1(x);

        int y_p2 = get_y_p1(y_p1);
        int y_p3 = get_y_p1(y_p2);
        int x_p2 = get_x_p1(x_p1);
        int x_p3 = get_x_p1(x_p2);

        // Setting other cells 
        m_current_state[x][y_p1] = true;
        m_current_state[x_p1][y_p1] = true;
        m_current_state[x_p1][y] = true;

        m_current_state[x_p2][y_p2] = true;
        m_current_state[x_p2][y_p3] = true;
        m_current_state[x_p3][y_p2] = true;
        m_current_state[x_p3][y_p3] = true;
    }

    for(int i = 0; i < m_height; i++)
    {
        for(int j =0; j<m_width;j++)
        {
            int idx = i*m_width + j;
            h_current[idx] = m_current_state[i][j];
            h_previous[idx] = m_previous_state[i][j];
            h_previous2[idx] = m_previous2_state[i][j];
        }
    }


}

// Methuselah pattern
void Game::add_methuselah(int x, int y)
{
    if(check_x(x) && check_y(y))
    {
        m_current_state[x][y] = true;

        // Getting other cells of the pattern
        int x_p1 = get_x_p1(x);
        int y_m1 = get_y_m1(y);
        int x_p2 = get_x_p1(x_p1);
        int y_p1 = get_y_p1(y);

        // Setting other cells
        m_current_state[x_p1][y_m1] = true;
        m_current_state[x_p1][y] = true;
        m_current_state[x_p2][y] = true;
        m_current_state[x_p2][y_p1] = true;
    }

    for(int i = 0; i < m_height; i++)
    {
        for(int j =0; j<m_width;j++)
        {
            int idx = i*m_width + j;
            h_current[idx] = m_current_state[i][j];
            h_previous[idx] = m_previous_state[i][j];
            h_previous2[idx] = m_previous2_state[i][j];
        }
    }


}

// Random adding
void Game::add_n_random(int n)
{
    std::random_device rd;
    std::mt19937 gen(rd()); // engine

    for(int i=0; i < n; i++)
    {
        std::uniform_int_distribution<> dist1(0, 3);
        int pattern_choice = dist1(gen);

        std::uniform_int_distribution<> dist2(0, m_height-1);
        int x_choice = dist2(gen);

        std::uniform_int_distribution<> dist3(0, m_width-1);
        int y_choice = dist3(gen);

        switch (pattern_choice)
        {
        case 0:
            add_glider(x_choice, y_choice);
            break;
        case 1:
            add_toad(x_choice, y_choice);
            break;
        case 2:
            add_beacon(x_choice, y_choice);
            break;
        case 3:
            add_methuselah(x_choice, y_choice);
            break;
        
        default:
            break;
        }


    }

}

// since we do want to keep the representation, we again cannot use openmp without having to make it linear
void Game::load(std::string file_name)
{
    std::ifstream input_file (file_name);

    if(!input_file.is_open())
        throw std::runtime_error("Couldn't open file: " + file_name);

    int height, width;
    input_file >> height;
    input_file >> width;

    std::vector<std::vector<bool>> state(height, std::vector<bool>(width));
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            int cellValue;
            input_file >> cellValue;
            state[i][j] = (cellValue == 1);
        }
    }
    m_current_state = state;
}

void Game::save(std::string file_name)
{
    std::string file = file_name + ".txt";
    std::ofstream outfile (file);

    outfile << m_height << std::endl;
    outfile << m_width << std::endl;
    for(int i = 0; i < m_height; i++)
    {
        for(int j = 0; j < m_width; j++)
        {
            if(m_current_state[i][j])
                outfile << "1 ";
            else
                outfile << "0 ";
        }
        outfile << std::endl;
    }

}

Game::~Game()
{

    if(m_stream) {
        cudaStreamSynchronize(m_stream);
        cudaStreamDestroy(m_stream);
        m_stream = nullptr;
    }

    // Clearing device memory
    if(d_current) {
        cudaFree(d_current);
        d_current = nullptr;
    }
    if(d_previous) {
        cudaFree(d_previous);
        d_previous = nullptr;
    }
    if(d_previous2) {
        cudaFree(d_previous2);
        d_previous2 = nullptr;
    }
    // Clearing pinned host memory
    if(h_current) {
        cudaFreeHost(h_current);
        h_current = nullptr;
    }
    if(h_previous) {
        cudaFreeHost(h_previous);
        h_previous = nullptr;
    }
    if(h_previous2) {
        cudaFreeHost(h_previous2);
        h_previous2 = nullptr;
    }

}
