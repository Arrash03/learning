#include <cstdint>
#include <random>
#include <cstring>
#include <iostream>
#include <immintrin.h>
#include <x86intrin.h>

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

#define quad_loop(start, step, end, ...)\
    for (; (start) + 4 * (step) - 1 < end; (start) += 4 * (step)) {\
        __VA_ARGS__\
        __VA_ARGS__\
        __VA_ARGS__\
        __VA_ARGS__\
    }

#define square_loop(start, step, end, ...)\
    for (; (start) + 2 * (step) - 1 < end; (start) += 2 * (step)) {\
        __VA_ARGS__\
        __VA_ARGS__\
    }

#define unrolling_loop(start, step, end, ...)\
    quad_loop(start, step, end, __VA_ARGS__)\
    square_loop(start, step, end, __VA_ARGS__)\
    if ((start) + (step) - 1 < (end)) {\
        __VA_ARGS__\
        (start) += (step);\
    }

bool avx512_buffer_equal(const uint64_t* a, const uint64_t* b, size_t size) {
    size_t i = 0;

    unrolling_loop(i, 8, size,
    {
        __m512i va = _mm512_loadu_si512((const void*)(a + i));
        __m512i vb = _mm512_loadu_si512((const void*)(b + i));
        __mmask64 cmp = _mm512_cmpeq_epi8_mask(va, vb);
        if (cmp != 0xFFFFFFFFFFFFFFFFULL)
        {
            return false;
        }
    });

    if (i + 3 < size)
    {
        __m256i va = _mm256_loadu_si256((__m256i*)(a + i));
        __m256i vb = _mm256_loadu_si256((__m256i*)(b + i));
        __mmask32 cmp = _mm256_cmpeq_epi64_mask(va, vb);
        if (cmp != 0xFFFFFFFF)
        {
            return false;
        }
        i += 4;
    }

    if (i + 1 < size)
    {
        __m128i va = _mm_loadu_si128((__m128i*)(a + i));
        __m128i vb = _mm_loadu_si128((__m128i*)(b + i));
        __mmask16 cmp = _mm_cmpeq_epi64_mask(va, vb);
        if (cmp != 0xFFFF)
        {
            return false;
        }
        i += 2;
    }
    
    if (i < size)
    {
        if (a[i] != b[i]) return false;
        i += 1;
    }

    return true;
}

bool __attribute__((noinline)) cmp(const uint64_t* a, const uint64_t* b, size_t size)
{
    return avx512_buffer_equal(a, b, size);
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
