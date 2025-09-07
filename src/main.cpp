#include <tbb/parallel_for.h>
#include <iostream>

int main() {
    tbb::parallel_for(0, 10, 1, [](int i){
        std::cout << "Hello from " << i << "\n";
    });
}
