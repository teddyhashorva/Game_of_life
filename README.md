# Game of life implementation in C++

This is a terminal based Game of Life implementation in C++. The evolution simulations are developed and tested in a Linux-Environment (Fedora), therefore they will most likely not work in other operating systems. Code is all written in C++, compilation is reccomended to be done via the CMakeLists.txt file, the script patch_cuda_headers.sh can be used to patch the CUDA version mismatches in your computer. One the application is running the function help in the terminal will provide a list of all possible functions the user can give, alongside a short description with what they do and an example of usage.

The computations can either be done in sequential form, or in parallel using the CUDA graphic cards. The difference in runtime is noticable, especially for larger game-states.

