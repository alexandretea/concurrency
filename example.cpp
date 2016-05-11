#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include "concurrency.hpp"

int
main()
{
    while (true) {
        tea::concurrency::Threadpool    tp(4);

        for (unsigned int i = 0; i < 10; ++i) {
            tp.push([](time_t, unsigned int i) -> bool {
                std::cout << std::this_thread::get_id() << ": " << i << std::endl;
                // usleep(2000);
                return true;
            }, time(NULL), i);
        }
    }
    return EXIT_SUCCESS;
}
