#include <cstdint>
#include <random>
#include <cstring>
#include <iostream>

static constexpr size_t size = 1ULL << 26;


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(0, std::numeric_limits<uint64_t>::max());

static void a(uint64_t ** src, uint64_t** dst)
{
    // *src = new uint64_t[size];
    // *dst = new uint64_t[size];
    *src = static_cast<uint64_t*>(std::aligned_alloc(64, size * sizeof(uint64_t)));
    *dst = static_cast<uint64_t*>(std::aligned_alloc(64, size * sizeof(uint64_t)));
    for(size_t i = 0; i < size; ++i)
    {
        (*src)[i] = distrib(gen);
    }

    std::memcpy(*dst, *src, size * sizeof(uint64_t));
    // Intentionally modify the last byte to create a difference
}

bool __attribute__((noinline)) cmp(const uint64_t* a, const uint64_t* b, size_t size)
{
    return std::memcmp(a, b, size * sizeof(uint64_t)) == 0;
}
int main()
{
    uint64_t* src;
    uint64_t* dst;
    a(&src, &dst);
    
    // Example usage of the comparison function
    bool result = cmp(dst, src, size);

    std::cout << result << std::endl;
    
    // Clean up
    delete [] src;
    delete [] dst;
}
