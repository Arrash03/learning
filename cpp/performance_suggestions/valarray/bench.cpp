#include <benchmark/benchmark.h>
#include <cstdint>
#include <vector>
// #include <array>
#include <valarray>
// #include <list>
// #include <numeric>
#include <vector>

// Benchmark size
static constexpr std::size_t N = 1 << 20;  // 1M elements

struct Internal
{
    std::string str;
    std::set<std::uint64_t> set;

    Internal& operator+=(const Internal& another)
    {
        str += another.str;
        set.insert(another.set.begin(), another.set.end());
        return *this;
    }

    Internal operator+(const Internal& another) const
    {
        Internal new_one(*this);
        new_one.str += another.str;
        new_one.set.insert(another.set.begin(), another.set.end());
        return new_one;
    }
};

struct Internal1
{
    uint64_t a;
    // uint64_t b;
    // uint64_t c;
    // uint64_t d;
    // uint64_t e;

    inline Internal1& operator+=(const Internal1& another)
    {
        a += another.a;
        // b += another.b;
        // c += another.c;
        // d += another.d;
        // e += another.e;
        return *this;
    }

    Internal1 operator+(const Internal1& another) const
    {
        Internal1 new_one(*this);
        new_one += another;
        return new_one;
    }
};

struct External
{
    // Internal inter;
    Internal1 inter;
    uint64_t a;
    uint8_t b;
    uint32_t c;

    External operator+(const External& another) const
    {
        External new_one(*this);
        new_one.inter += another.inter;
        new_one.a += another.a;
        new_one.b += another.b;
        new_one.c += another.c;
        return new_one;
    };
};

// External std::operator+(const )

// Helper to fill containers with sample data
static void fill_data(auto& a, auto& b) {
    uint64_t i = 0;
    for (auto& elem: a)
    {
        elem.a = i++;
        elem.b = i++;
        elem.c = i++;
    }
    i = 187;
    for (auto& elem: b)
    {
        elem.a = i++;
        elem.b = i++;
        elem.c = i++;
    }
}

// 1) std::vector<int> addition
static void BM_VectorAdd(benchmark::State& state) {
    std::vector<External> a(N), b(N), c(N);
    fill_data(a, b);  // initialize data
    for (auto _ : state) {
        for (std::size_t i = 0; i < N; ++i) {
            c[i] = a[i] + b[i];
        }
        // benchmark::DoNotOptimize(a);
        // benchmark::DoNotOptimize(b);
        // benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_VectorAdd)->Unit(benchmark::kMicrosecond);

// // 2) std::array<std::uint_fast32_t, N> addition
// static void BM_ArrayAdd(benchmark::State& state) {
//     std::array<std::uint_fast32_t, N> a, b, c;
//     fill_data(a, b);
//     for (auto _ : state) {
//         for (std::size_t i = 0; i < N; ++i) {
//             c[i] = a[i] + b[i];
//         }
//         benchmark::DoNotOptimize(a);
//         benchmark::DoNotOptimize(b);
//         benchmark::DoNotOptimize(c);
//     }
// }
// BENCHMARK(BM_ArrayAdd)->Unit(benchmark::kMicrosecond);

// 3) std::valarray<std::uint_fast32_t> addition
static void BM_ValarrayAdd(benchmark::State& state) {
    std::valarray<External> a(N), b(N), c(N);
    fill_data(a, b);
    for (auto _ : state) {
        c = a + b;  // element-wise vectorized
        // benchmark::DoNotOptimize(a);
        // benchmark::DoNotOptimize(b);
        // benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_ValarrayAdd)->Unit(benchmark::kMicrosecond);

// Automatically generate main()
BENCHMARK_MAIN();
