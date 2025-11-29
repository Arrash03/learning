#include <immintrin.h>  // AVX
#include <benchmark/benchmark.h>
#include <random>

static size_t container_size = 1'024;
static constexpr size_t iterations = 10'000;
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(1, 1'000'000'000'000);

void fill_data(uint64_t *container)
{
    for (size_t i = 0; i < container_size; ++i)
    {
        container[i] = distrib(gen);
    }
}

static bool sum_128(auto& a, auto& b,  auto& result)
{
    size_t i = 0;
    while (i < container_size - 1)
    {
        __m128i_u va = _mm_loadu_epi64((__m128i_u*)&a[i]);       // Load 2 ints into AVX register
        __m128i_u vb = _mm_loadu_epi64((__m128i_u*)&b[i]);
        __m128i_u vr = _mm_add_epi64(va, vb);   // Parallel addition of 2 ints
        _mm_storeu_epi64((__m128i_u*)&result[i], vr);
        i += 2;
    }

    while (i < container_size)
    {
        result[i] = a[i] + b[i];
        i += 1;
    }

    return true;
}

static bool sum_256(auto& a, auto& b,  auto& result)
{
    size_t i = 0;
    while (i < container_size - 3)
    {
        __m256i va = _mm256_loadu_epi64(&a[i]);       // Load 4 ints into AVX register
        __m256i vb = _mm256_loadu_epi64(&b[i]);
        __m256i vr = _mm256_add_epi64(va, vb);   // Parallel addition of 4 ints
        _mm256_storeu_epi64(&result[i], vr);
        i += 4;
    }

    while (i < container_size)
    {
        result[i] = a[i] + b[i];
        i += 1;
    }

    return true;
}

static bool sum_512(auto& a, auto& b,  auto& result)
{
    size_t i = 0;
    while (i < container_size - 7)
    {
        __m512i va = _mm512_loadu_epi64(&a[i]);       // Load 8 ints into AVX register
        __m512i vb = _mm512_loadu_epi64(&b[i]);
        __m512i vr = _mm512_add_epi64(va, vb);   // Parallel addition of 8 ints
        _mm512_storeu_epi64(&result[i], vr);
        i += 8;
    }

    while (i < container_size)
    {
        result[i] = a[i] + b[i];
        i += 1;
    }

    return true;
}

static void avx_512(benchmark::State& state)
{
    container_size = state.range(0);
    uint64_t* a = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* b = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* result = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    benchmark::DoNotOptimize(a);
    benchmark::DoNotOptimize(b);
    benchmark::DoNotOptimize(result);
    fill_data(a);
    fill_data(b);
    
    for (auto _ : state)
    {
        bool sum_result = sum_512(a, b, result);
        benchmark::DoNotOptimize(sum_result);
    }
}

static void avx_256(benchmark::State& state)
{
    container_size = state.range(0);
    uint64_t* a = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* b = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* result = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    benchmark::DoNotOptimize(a);
    benchmark::DoNotOptimize(b);
    benchmark::DoNotOptimize(result);
    fill_data(a);
    fill_data(b);
    
    for (auto _ : state)
    {
        bool sum_result = sum_256(a, b, result);
        benchmark::DoNotOptimize(sum_result);
    }
}

static void avx_128(benchmark::State& state)
{
    container_size = state.range(0);
    uint64_t* a = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* b = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* result = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    benchmark::DoNotOptimize(a);
    benchmark::DoNotOptimize(b);
    benchmark::DoNotOptimize(result);
    fill_data(a);
    fill_data(b);
    
    for (auto _ : state)
    {
        bool sum_result = sum_128(a, b, result);
        benchmark::DoNotOptimize(sum_result);
    }
}


static bool sum(auto& a, auto& b,  auto& result)
{
    for(size_t i=0; i < container_size; i++)
        result[i] = a[i] + b[i];
    return true;
}

static void normal_sum(benchmark::State& state)
{
    container_size = state.range(0);
    
    uint64_t* a = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* b = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    uint64_t* result = (uint64_t*)std::aligned_alloc(64, container_size * sizeof(uint64_t));
    benchmark::DoNotOptimize(a);
    benchmark::DoNotOptimize(b);
    benchmark::DoNotOptimize(result);
    fill_data(a);
    fill_data(b);
    
    for (auto _ : state)
    {
        bool sum_result = sum(a, b, result);
        benchmark::DoNotOptimize(sum_result);
    }
}

BENCHMARK(avx_512)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<10)
    ->Arg(1<<15)
    ->Arg(1<<20);

BENCHMARK(avx_256)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<10)
    ->Arg(1<<15)
    ->Arg(1<<20);

BENCHMARK(avx_128)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<10)
    ->Arg(1<<15)
    ->Arg(1<<20);

BENCHMARK(normal_sum)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<10)
    ->Arg(1<<15)
    ->Arg(1<<20);

BENCHMARK_MAIN();
