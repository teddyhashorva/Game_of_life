#ifndef CLI_H
#define CLI_H
#include <iostream>
#include <string>
#include <vector>
class Game;

class Cli
{
    private:
        bool m_check = false;
        bool m_print = true;

        bool m_useCUDA = false;

        int m_delay_ms = 700;
        std::string m_begin_output = "”\033[2J\033[H";
        std::string m_alternate_screen_mode = "”\033[?1049h";
        std::string m_end_alternate_screen = "”\033[?1049l";
        Game* m_game = nullptr;


        std::vector<std::string> tokenize(std::string& input);


    public:
        Cli();

        void help()
        {
            std::cout << "Functions that you can use are: " << std::endl;
            std::cout << "create x y        -> create a world with dimensions x and y." << std::endl;
            std::cout << "load <filename>   -> load game-state from the file name.    " << std::endl;
            std::cout << "save <filename>   -> save the current world-state in the file name" << std::endl;
            std::cout << "delay x           -> set the delay to x ns. By default 700ms" << std::endl;
            std::cout << "run n             -> run the simulation n times." << std::endl;
            std::cout << "set x y <0/1>     -> set the cell at indexes x, y to alive(1) or dead(0)." << std::endl;
            std::cout << "set x <0/1>       -> set the cell number x to alive(1) or dead(0)" << std::endl;
            std::cout << "get x y           -> get the status of the cell at indexes x,y" << std::endl;
            std::cout << "get x             -> get the status of the cell number x" << std::endl;
            std::cout << "glider x y        -> set a glider pattern at index x, y" << std::endl;
            std::cout << "toad x y          -> set a toad pattern at index x, y" << std::endl;
            std::cout << "beacon x y        -> set a beacon pattern at index x, y" << std::endl;
            std::cout << "methuselah x y    -> set a methuselah pattern at index x, y" << std::endl;
            std::cout << "random n          -> add n random patterns" << std::endl;
            std::cout << "print <1/0>       -> set print to true(1) or false(0), by default true" << std::endl;
            std::cout << "stability <1/0>   -> set stability check to true(1) or false(0), by default false" << std::endl;
            std::cout << "help              -> get a list of the helpful commands you can use" << std::endl;
            std::cout << "cuda <1/0>        -> 1 if CUDA should be used, 0 if Openmp. By default openmp is used" << std::endl;
            std::cout << "linear            -> runs 500 000 iterations with the linear implementation of evolve" << std::endl;
            std::cout << "quit              -> ends this programme." << std::endl;

        }
        void welcome()
        {
            std::cout << "******************************************" << std::endl;
            std::cout << "*          Welcome to Game of Life       *" << std::endl;
            std::cout << "******************************************" << std::endl;
            help();

        }

        // Lifetime function of the CLI
        void active();
        double run(int n);

};
#endif