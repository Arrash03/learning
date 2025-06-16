#include <immintrin.h>  // AVX
#include <benchmark/benchmark.h>
#include <iostream>
#include <random>

static constexpr size_t container_size = 8;
static constexpr size_t iterations = 10'000'000'000;
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(1, 1'000'000'000'000);

void fill_data(auto& container)
{
    for (auto& elem: container)
    {
        elem = distrib(gen);
    }
}


static void simd_sum(benchmark::State& state)
{
    alignas(64) uint64_t a[container_size];
    alignas(64) uint64_t b[container_size];
    fill_data(a);
    fill_data(b);
    alignas(64) uint64_t result[container_size];

    for (auto _ : state)
    {
        __m512 va = _mm512_load_ps(a);       // Load 8 ints into AVX register
        __m512 vb = _mm512_load_ps(b);
        __m512 vr = _mm512_add_ps(va, vb);   // Parallel addition of 8 ints
        _mm512_store_ps(result, vr);         // Store result back to array
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(simd_sum)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

static void normal_sum(benchmark::State& state)
{
    alignas(64) uint64_t a[container_size];
    alignas(64) uint64_t b[container_size];
    fill_data(a);
    fill_data(b);
    alignas(64) uint64_t result[container_size];

    for (auto _ : state)
    {
        for(size_t i=0; i < container_size; i++)
            result[i] = a[i] + b[i];
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(normal_sum)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK_MAIN();
