#include <cstddef>
#include <random>
#include <xsimd/xsimd.hpp>
#include <benchmark/benchmark.h>
#include <immintrin.h>  // AVX

static constexpr size_t container_size = 64;
static constexpr size_t iterations = 1'000;
static constexpr size_t size = 1ULL << 20;

using cstr = uint64_t *;
using batch = xsimd::batch<uint64_t, xsimd::best_arch>;
using batch_bool = xsimd::batch_bool<uint64_t, xsimd::best_arch>;

#define work(i)\
do {\
    __m512i va = _mm512_loadu_si512((const void*)(a + (i)));\
    __m512i vb = _mm512_loadu_si512((const void*)(b + (i)));\
    __mmask64 cmp = _mm512_cmpeq_epi8_mask(va, vb);\
    if (cmp != 0xFFFFFFFFFFFFFFFFULL)\
    {\
        return false;\
    }\
} while (0);

#define quad_loop(start, step, end, tmp)\
    for (; (start) + 4 * (step) - 1 < end; (start) += 4 * (step)) {\
        work((start))\
        work((start) + (step))\
        work((start) + 2 * (step))\
        work((start) + 3 * (step))\
    }

#define square_loop(start, step, end, ...)\
    for (; (start) + 2 * (step) - 1 < end; (start) += 2 * (step)) {\
        work((start))\
        work((start) + (step))\
    }

#define unrolling_loop(start, step, end)\
    quad_loop(start, step, end, tmp)\
    square_loop(start, step, end, work(i))\
    if ((start) + (step) - 1 < (end)) {\
        work((start))\
        (start) += (step);\
    }

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(-128, 127);

static void a(uint64_t ** src, uint64_t** dst)
{
    *src = static_cast<uint64_t*>(std::aligned_alloc(64, size * sizeof(uint64_t)));
    *dst = static_cast<uint64_t*>(std::aligned_alloc(64, size * sizeof(uint64_t)));
    for(size_t i = 0; i < size; ++i)
    {
        (*src)[i] = distrib(gen);
    }

    std::memcpy(*dst, *src, size * sizeof(uint64_t));
}

static void b(uint64_t** src, uint64_t** dst)
{
    delete [] (*src);
    delete [] (*dst);
}

// I used avx512 register in intel cpu processor and it was just a simple test and may be for different arch you see
// different resutlts
static bool xsimd_cmp(cstr dst, cstr src, size_t size)
{
    size_t i = 0;

    while (size - i > 8)
    {
        batch va = batch::load_unaligned(src + i);
        batch vb = batch::load_unaligned(dst + i);
        batch_bool vc = va != vb;
        bool buffer[batch::size];
        benchmark::DoNotOptimize(buffer);
        vc.store_aligned(&buffer[0]);
        for(size_t j = 0; j < batch::size; ++j)
        {
            if (__builtin_expect(!!(buffer[j]), 0))
                return false;
        }
        i += 8;
    }

    for(size_t j = i; j < size; ++j)
    {
        if (src[j] != dst[j])
            return false;
    }

    return true;
}

static void xsimd_function(benchmark::State& state)
{
    uint64_t* src;
    uint64_t* dst;
    a(&src, &dst);
    for(auto _: state)
    {
        int a = xsimd_cmp(dst, src, size);
        benchmark::DoNotOptimize(a);
    }
    benchmark::ClobberMemory();
    b(&src, &dst);
}


static void std_function(benchmark::State& state)
{
    uint64_t* src;
    uint64_t* dst;
    a(&src, &dst);
    for(auto _: state)
    {
        int a = std::memcmp(src, dst, size * sizeof(uint64_t)) == 0;
        benchmark::DoNotOptimize(a);
    }
    benchmark::ClobberMemory();
    b(&src, &dst);
}

static bool cmp(const uint64_t* a, const uint64_t* b, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        auto result = a[i] != b[i];
        benchmark::DoNotOptimize(result);
        if (result)
            return false;
    }
    return true;
}

static void normal_function(benchmark::State& state)
{
    uint64_t* src;
    uint64_t* dst;
    a(&src, &dst);
    for(auto _: state)
    {
        int a = cmp(src, dst, size);
        benchmark::DoNotOptimize(a);
    }
    benchmark::ClobberMemory();
    b(&src, &dst);
}

bool avx512_buffer_equal(const uint64_t* a, const uint64_t* b, size_t size) {
    size_t i = 0;

    unrolling_loop(i, 8, size);

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

// Example usage in benchmark
static void avx512_simd_function(benchmark::State& state)
{
    uint64_t* src;
    uint64_t* dst;
    a(&src, &dst);
    for(auto _: state)
    {
        int res = avx512_buffer_equal(src, dst, size);
        benchmark::DoNotOptimize(res);
    }
    benchmark::ClobberMemory();
    b(&src, &dst);
}

// Register the new benchmark
BENCHMARK(avx512_simd_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK(std_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK(normal_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK(xsimd_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(12);


BENCHMARK_MAIN();
