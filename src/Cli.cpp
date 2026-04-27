#include "../include/Cli.h"
#include "../include/Game.h"
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>



Cli::Cli() = default;

std::vector<std::string> Cli::tokenize(std::string& input)
{
    std::istringstream iss(input);
    std::vector <std::string> tokens;
    std::string word;

    while(iss >> word)
        tokens.push_back(word);
    return tokens;
}

double Cli::run(int n)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << m_alternate_screen_mode;

    if(m_useCUDA)
    {
        m_game->evolveCUDA(n, m_print, m_delay_ms);
    }

    else
    {
        for(int i=0; i < n; i++)
        {
            std::cout << m_begin_output;
            m_game->evolve();
            if(m_print)
                m_game->print();
            if(m_check)
                std::cout << "Is the gamestate stable: " << m_game->is_stable() << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(m_delay_ms));
        }
    }

    std::cout << m_end_alternate_screen;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    // Update after duration calc so that it doesn't affect the runtime
    if(m_useCUDA)
        m_game->updateCUDA(true);

    return duration.count();

}

void Cli::active()
{
    welcome();
    std::string input;
    while(true)
    {
        std::cout << "> ";
        std::getline(std::cin, input);

        if(input.empty()) continue;

        auto tokens = tokenize(input);

        std::string cmd = tokens[0];

        // switch doesn't work with strings
        if(cmd == "create")
        {
            int height = std::stoi(tokens[1]);
            int width = std::stoi(tokens[2]);

            std::vector<bool> row (width);
            std::vector<std::vector<bool>> current_state (height, row);
            
            std::vector<std::vector<bool>> prev_state (height, row);
            std::vector<std::vector<bool>> prev2_state (height, row);

            // creatin the game-states
            for(int i = 0; i < height; i++)
            {

                for(int j=0; j<width; j++)
               {
                    current_state.at(i).at(j) = false;
                    prev_state.at(i).at(j) = false;
                    prev2_state.at(i).at(j) = false;
                }
            }
            
           if(m_game != nullptr)
                delete m_game;

            m_game = new Game(height, width, current_state, prev_state, prev2_state);
            std::cout << "Game created with the given dimensions!" << std::endl;
        }

        

        else if(cmd == "load")
        {
            std::string file_name = tokens[1];
            if(m_game != nullptr)
                delete m_game;
            m_game = new Game(file_name);
        }


        else if(cmd == "save")
        {
            if(m_game)
            {
                std::string file_name = tokens[1];
                m_game->save(file_name);
            }
            else
                std::cout << "Please create a game before you export" << std::endl;
        }
            

        else if(cmd == "delay")
        {
            int delay = std::stoi(tokens[1]);
            m_delay_ms = delay;
        }


        else if(cmd == "run")
        {
            if(m_game)
            {
                int n = std::stoi(tokens[1]);
                double duration = run(n);
                std::cout << "Duration: " << duration << std::endl;
            }
            else
                std::cout << "Please create a Game instance before running";            
            
        }

        else if(cmd == "linear")
        {

            auto start = std::chrono::high_resolution_clock::now();

            for(int i = 0; i < 500'000; ++i)
            {
                m_game->evolveLinear();
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;

            std::cout << "Runtime linear: " << duration.count() << " in ms" << std::endl;
        }



        else if(cmd == "set")
        {
            if(m_game)
            {
                int x = std::stoi(tokens[1]);
                if(tokens.size()==4)
                {
                    int y = std::stoi(tokens[2]);
                    if(tokens[3]=="1")                    
                        m_game->insert_cell(x, y, true);
                        
                    else                    
                        m_game->insert_cell(x, y, false);

                }
                else
                {
                    if(tokens[2]=="1")
                        m_game->insert_cell(x, true);
                    else
                        m_game->insert_cell(x, false);
                }
            }
            else
                std::cout << "Please create a game instace before applying this command" << std::endl;
        }



        else if(cmd == "get")
        {
            if(m_game)
            {
                int x = std::stoi(tokens[1]);
                if(tokens.size() == 3)
                {
                    int y = std::stoi(tokens[2]);
                    if(m_game->get_cell(x, y))
                        std::cout << "Alive" << std::endl;
                    else
                        std::cout << "Dead" << std::endl;
                }
                else
                {
                    if(m_game->get_cell(x))
                        std::cout << "Alive" << std::endl;
                    else
                        std::cout << "Dead" << std::endl;
                }
            }
            else
                std::cout << "please create a Game instance before getting cell states" << std::endl;

        }


        else if(cmd == "glider")
        {
            if(m_game)
            {
                int x = std::stoi(tokens[1]);
                int y = std::stoi(tokens[2]);
                m_game->add_glider(x, y);
            }
            else
                std::cout << "please create a Game instance before adding!" << std::endl;
        }
            
        else if(cmd == "toad")
        {
            if(m_game)
            {
                int x = std::stoi(tokens[1]);
                int y = std::stoi(tokens[2]);
                m_game->add_toad(x, y);
            }
            else
                std::cout << "please create a Game instance before adding!" << std::endl;
        }


        else if(cmd == "beacon")
        {
            if(m_game)
            {
                int x = std::stoi(tokens[1]);
                int y = std::stoi(tokens[2]);
                m_game->add_beacon(x, y);
            }
            else
                std::cout << "please create a Game instance before adding!" << std::endl;
        }


        else if(cmd == "methuselah")
        {
            if(m_game)
            {
                int x = std::stoi(tokens[1]);
                int y = std::stoi(tokens[2]);
                m_game->add_methuselah(x, y);
            }
            else
                std::cout << "please create a Game instance before adding!" << std::endl;
        }


        else if(cmd == "random")
        {
            if(m_game)
            {
                int n = std::stoi(tokens[1]);
                m_game->add_n_random(n);
            }
            else
                std::cout << "please create a Game instance before adding!" << std::endl;
        }



        else if(cmd == "print")
        {
            if(tokens[1] == "1")
                m_print = true;
            else if(tokens[1] == "0")
                m_print = false;
        }


        else if(cmd == "stability")
        {
            if(tokens[1] == "1")
                m_check = true;
            else if(tokens[1] == "0")
                m_check = false;
        }

        else if(cmd == "cuda")
        {
            if(tokens[1] == "1")
                m_useCUDA = true;
            else if(tokens[1] == "0")
                m_useCUDA = false;
        }
            
        else if(cmd == "help")
            help();

        else if(cmd == "quit")
            return;

        else
            std::cout << "Please use a correct command. help gives a list of the functions you can use." << std::endl;

    }

}
