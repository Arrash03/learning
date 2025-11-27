#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <emmintrin.h>
#include <random>
#include <benchmark/benchmark.h>
#include <immintrin.h>
#include <sys/types.h>

static constexpr size_t iterations = 1'000'00;
static constexpr size_t rows = 256;
static constexpr size_t columns = 1500;

struct Rule
{
    size_t offset;
    size_t vlaue;
};

static volatile Rule rule{
    .offset = 450,
    .vlaue = 45
};


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(0, 127);

using array = uint64_t[columns];
using td_array = uint64_t(*)[columns];

static void fill_array(array* b, const size_t size)
{
    for(size_t i = 0; i < size; ++i)
        (*b)[i] = distrib(gen);
}

static void fill_td_array(td_array* arr1, const size_t x_size)
{
    *arr1 = (td_array)std::aligned_alloc(8, x_size * columns * sizeof(uint64_t));
    for (size_t i = 0; i < x_size; ++i)
    {
        fill_array(&(*arr1)[i], columns);
    }
}

static void delete_buffer(td_array b1)
{
    free (b1);
}

static bool match2d_array_normal(const td_array a, size_t size, const volatile Rule& rule)
{
    for (size_t i = 0; i < size; ++i)
    {
        auto res = a[i][rule.offset] != rule.vlaue;
        benchmark::DoNotOptimize(res);
    }
    return true;
}

static void normal_function(benchmark::State& state)
{
    benchmark::DoNotOptimize(rule);

    td_array b;
    fill_td_array(&b, rows);
    for(auto _: state)
    {
        int a = match2d_array_normal(b, rows, rule);
        benchmark::DoNotOptimize(a);
    }
    benchmark::ClobberMemory();
    delete_buffer(b);
}

bool match_simd_array_normal(const td_array a, size_t size, const volatile Rule& rule)
{
    size_t i = 0;

    while (i < size - 7)
    {
        __m512i va = _mm512_set1_epi64(rule.vlaue);
        __m512i vb = _mm512_set_epi64(a[i][rule.offset], a[i + 1][rule.offset], a[i + 2][rule.offset], a[i + 3][rule.offset],
                    a[i + 4][rule.offset], a[i + 5][rule.offset], a[i + 6][rule.offset], a[i + 7][rule.offset]);
        __mmask64 cmp = _mm512_cmpeq_epi8_mask(va, vb);
        benchmark::DoNotOptimize(cmp);
        i += 8;
    }

    while (i < size)
    {
        if (a[i][rule.offset] != rule.vlaue) return false;
        i += 1;
    }

    return true;
}

static void avx512_simd_function(benchmark::State& state)
{
    benchmark::DoNotOptimize(rule);

    td_array b;
    fill_td_array(&b, rows);
    for(auto _: state)
    {
        int a = match_simd_array_normal(b, rows, rule);
        benchmark::DoNotOptimize(a);
    }
    benchmark::ClobberMemory();
    delete_buffer(b);
}

// Register the new benchmark
BENCHMARK(avx512_simd_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK(normal_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK_MAIN();
