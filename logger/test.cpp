#include <iostream>
#include <thread>

#include "logger.h"

void thread_func(int idx) {
    for (size_t i = 0; i < 300; i++) {
        LOG_INFO << "thread " << idx << " " << i;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    LOG_INFO << "hello logger";

    const int NUM_THREADS = 5;
    std::thread threads[NUM_THREADS];
    // create threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i] = std::thread(thread_func, i);
    }
    // wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i].join();
    }
    std::cout << "All threads finished." << std::endl;

    return 0;
}