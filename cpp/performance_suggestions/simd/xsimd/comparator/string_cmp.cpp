#include <cstddef>
#include <xsimd/xsimd.hpp>
#include <benchmark/benchmark.h>

static constexpr size_t container_size = 64;
static constexpr size_t iterations = 1'000'000;

using cstr = const char * const;
using batch = xsimd::batch<uint64_t>;
using batch_bool = xsimd::batch_bool<uint64_t>;

static cstr src = "sdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkj";
static cstr dst = "sdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkjsdfkljsfkdjskljfdlksjfkj";
static constexpr size_t size = 433;


// I used avx512 register in intel cpu processor and it was just a simple test and may be for different arch you see
// different resutlts
static bool simd_cmp(cstr dst, cstr src, size_t size)
{
    size_t i = 0;

    while (size - (i * 64) > 64)
    {
        batch va = batch::load_unaligned(src + i * 64);
        batch vb = batch::load_unaligned(dst + i * 64);
        batch_bool vc = va != vb;
        for(size_t i = 0; i < batch_bool::size; ++i)
        {
            if (vc.get(i))
                return false;
        }
        ++i;
    }

    for(size_t j = i*64; j < size; ++j)
    {
        if (src[j] != dst[j])
            return false;
    }

    return true;
}

static void simd_function(benchmark::State& state)
{
    for(auto _: state)
    {
        simd_cmp(dst, src, size);
    }
}

BENCHMARK(simd_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

static void normal_function(benchmark::State& state)
{
    for(auto _: state)
    {
        int a = std::memcmp(src, dst, size);
        benchmark::DoNotOptimize(a);
    }
}

BENCHMARK(normal_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations);

BENCHMARK_MAIN();
