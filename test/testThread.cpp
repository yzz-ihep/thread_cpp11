#include <iostream>

#include "thread.h"

int main(void)
{
    Thread t("test", []() {
        std::cout << "Thread name is " << getCurrentThreadName() << std::endl;
    });

    t.start();
    t.join();

    return 0;
}
