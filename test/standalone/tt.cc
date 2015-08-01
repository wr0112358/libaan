#include <chrono>

int main()
{
    const auto t = std::chrono::high_resolution_clock::from_time_t(-2<<31);
    (void)t;
}
