#include <cstdint>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(1, 1'000'000'000'000);

using a = uint64_t[8];

void __attribute__((noinline)) fill_data(a& container)
{
    for (auto& elem: container)
    {
        elem = distrib(gen);
    }
}