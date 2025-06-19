#include <cstdint>
#include <xsimd/xsimd.hpp>

#include <benchmark/benchmark.h>

static constexpr size_t container_size = 8;
static constexpr size_t iterations = 1'000'000'000;

using a = uint64_t[container_size];

extern void __attribute__((noinline)) fill_data(a& container);

static void simd_sum(benchmark::State& state)
{
    using batch = xsimd::batch<uint64_t>;
    uint64_t a[container_size];
    uint64_t b[container_size];
    fill_data(a);
    fill_data(b);
    uint64_t result[container_size];

    for (auto _ : state)
    {
        batch va = batch::load_unaligned(a);
        batch vb = batch::load_unaligned(b);
        batch vc = va + vb;
        vc.store_unaligned(result);
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(simd_sum)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

static void normal_sum(benchmark::State& state)
{
    uint64_t a[container_size];
    uint64_t b[container_size];
    fill_data(a);
    fill_data(b);
    uint64_t result[container_size];

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
