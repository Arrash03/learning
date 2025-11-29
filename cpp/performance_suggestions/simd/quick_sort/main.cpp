#include <cstddef>
#include <cstring>
#include <cstdint>
#include <vector>
#include <immintrin.h>
#include <cstdlib>
#include <random>
#include <benchmark/benchmark.h>
#include <algorithm>

static constexpr size_t iterations = 1'000;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint64_t> distrib(0, 1ULL<<40);

static void fill_array(uint64_t* b, const size_t size)
{
    for(size_t i = 0; i < size; ++i)
        b[i] = distrib(gen);
}


bool quick_sort_normal(uint64_t* arr, int left, int right) {
    if (left >= right) return true;

    uint64_t pivot = arr[right];
    std::vector<std::pair<int, int>> stack;
    stack.emplace_back(left, right);

    while (!stack.empty()) {
        auto [start, end] = stack.back();
        stack.pop_back();

        if (start >= end) continue;

        uint64_t pivot = arr[end];
        int lt = start;
        int gt = end;
        int i = start;

        while (i <= gt) {
            if (arr[i] < pivot) {
                std::swap(arr[i], arr[lt]);
                lt++;
                i++;
            } else if (arr[i] > pivot) {
                std::swap(arr[i], arr[gt]);
                gt--;
            } else {
                i++;
            }
        }

        stack.emplace_back(start, lt - 1);
        stack.emplace_back(gt + 1, end);
    }

    return true;
}

void compare_512(uint64_t* src, uint64_t* dst, int& i, int& lt, int& gt, uint64_t pivot)
{
    size_t dl = lt, dr = gt, de = 0;
    while (i + 7 <= gt)
    {
        __m512i va = _mm512_loadu_epi64((const void*)(&src[i]));
        __m512i vb = _mm512_set1_epi64(pivot);

        __mmask8 gm = _mm512_cmp_epu64_mask(va, vb, _MM_CMPINT_GT); // a > b?
        __mmask8 lm = _mm512_cmp_epu64_mask(va, vb, _MM_CMPINT_LT); // a < b?

        for (int j = 0; j < 8; ++j) {
            bool is_gt = (gm >> j) & 1;
            bool is_lt = (lm >> j) & 1;
            if (is_gt)
            {
                dst[dr--] = src[i + j];
                // std::swap(src[i], src[gt]);
                // gt--;
            }
            else if (is_lt)
            {
                dst[dl++] = src[i + j];
                // std::swap(src[i], src[lt]);
                // lt++;
                // i++;
            }
            else
            {
                std::swap(dst[dr--], dst[gt - de]);
                dst[gt - de] = src[i + j];
                de++;
                // i++;
            }
        }
        i += 8;
    }

    while (i <= gt) {
        if (src[i] < pivot) {
            dst[dl++] = src[i];
            // std::swap(src[i], src[lt]);
            // lt++;
            // i++;
        } else if (src[i] > pivot) {
            dst[dr--] = src[i];
            // std::swap(src[i], src[gt]);
            // gt--;
        } else {
            std::swap(dst[dr--], dst[gt - de]);
            dst[gt - de] = src[i];
            de++;
        }
        i++;
    }

    while (de > 0)
    {
        std::swap(dst[++dr], dst[gt - de + 1]);
        de--;
    }

    std::memcpy(src + lt, dst + lt, (gt - lt + 1) * sizeof(uint64_t));

    lt = dl;
    gt = dr;
}

bool avx512_quick_sort(uint64_t* arr, int left, int right) {
    if (left >= right) return true;

    uint64_t pivot = arr[right];
    uint64_t *dst = new uint64_t[right + 1];
    std::vector<std::pair<int, int>> stack;
    stack.reserve(right + 1);
    stack.emplace_back(left, right);

    while (!stack.empty()) {
        auto [start, end] = stack.back();
        stack.pop_back();

        if (start >= end) continue;

        uint64_t pivot = arr[end];
        int lt = start;
        int gt = end;
        int i = start;

        compare_512(arr, dst, i, lt, gt, pivot);

        stack.emplace_back(start, lt - 1);
        stack.emplace_back(gt + 1, end);
    }

    return true;
}

static void avx512_function(benchmark::State& state)
{
    size_t size = state.range(0);
    
    uint64_t *arr = (uint64_t*)std::aligned_alloc(8, size * sizeof(uint64_t));
    benchmark::DoNotOptimize(arr);
    fill_array(arr, size);

    for(auto _: state)
    {
        bool result = avx512_quick_sort(arr, 0, size - 1);
        benchmark::DoNotOptimize(result);
    }
    benchmark::ClobberMemory();
}

void compare_256(uint64_t* src, uint64_t* dst, int& i, int& lt, int& gt, uint64_t pivot)
{
    size_t dl = lt, dr = gt, de = 0;
    while (i + 7 <= gt)
    {
        __m256i va = _mm256_loadu_epi64((const void*)(&src[i]));
        __m256i vb = _mm256_set1_epi64x(pivot);

        __mmask8 gm = _mm256_cmp_epu64_mask(va, vb, _MM_CMPINT_GT); // a > b?
        __mmask8 lm = _mm256_cmp_epu64_mask(va, vb, _MM_CMPINT_LT); // a < b?

        for (int j = 0; j < 4; ++j) {
            bool is_gt = (gm >> j) & 1;
            bool is_lt = (lm >> j) & 1;
            if (is_gt)
            {
                dst[dr--] = src[i + j];
                // std::swap(src[i], src[gt]);
                // gt--;
            }
            else if (is_lt)
            {
                dst[dl++] = src[i + j];
                // std::swap(src[i], src[lt]);
                // lt++;
                // i++;
            }
            else
            {
                std::swap(dst[dr--], dst[gt - de]);
                dst[gt - de] = src[i + j];
                de++;
                // i++;
            }
        }
        i += 4;
    }

    while (i <= gt) {
        if (src[i] < pivot) {
            dst[dl++] = src[i];
            // std::swap(src[i], src[lt]);
            // lt++;
            // i++;
        } else if (src[i] > pivot) {
            dst[dr--] = src[i];
            // std::swap(src[i], src[gt]);
            // gt--;
        } else {
            std::swap(dst[dr--], dst[gt - de]);
            dst[gt - de] = src[i];
            de++;
        }
        i++;
    }

    while (de > 0)
    {
        std::swap(dst[++dr], dst[gt - de + 1]);
        de--;
    }

    std::memcpy(src + lt, dst + lt, (gt - lt + 1) * sizeof(uint64_t));

    lt = dl;
    gt = dr;
}

bool avx256_quick_sort(uint64_t* arr, int left, int right) {
    if (left >= right) return true;

    uint64_t pivot = arr[right];
    uint64_t *dst = new uint64_t[right + 1];
    std::vector<std::pair<int, int>> stack;
    stack.reserve(right + 1);
    stack.emplace_back(left, right);

    while (!stack.empty()) {
        auto [start, end] = stack.back();
        stack.pop_back();

        if (start >= end) continue;

        uint64_t pivot = arr[end];
        int lt = start;
        int gt = end;
        int i = start;

        compare_256(arr, dst, i, lt, gt, pivot);

        stack.emplace_back(start, lt - 1);
        stack.emplace_back(gt + 1, end);
    }

    return true;
}

static void avx256_function(benchmark::State& state)
{
    size_t size = state.range(0);
    
    uint64_t *arr = (uint64_t*)std::aligned_alloc(8, size * sizeof(uint64_t));
    benchmark::DoNotOptimize(arr);
    fill_array(arr, size);

    for(auto _: state)
    {
        bool result = avx256_quick_sort(arr, 0, size - 1);
        benchmark::DoNotOptimize(result);
    }
    benchmark::ClobberMemory();
}

static void normal_function(benchmark::State& state)
{
    size_t size = state.range(0);
    
    uint64_t *arr = (uint64_t*)std::aligned_alloc(8, size * sizeof(uint64_t));
    benchmark::DoNotOptimize(arr);
    fill_array(arr, size);

    for(auto _: state)
    {
        bool result = quick_sort_normal(arr, 0, size - 1);
        benchmark::DoNotOptimize(result);
    }
    benchmark::ClobberMemory();
}

static bool std_sort(uint64_t *start, uint64_t* end)
{
    std::sort(start, end);
    return true;
}

static void std_function(benchmark::State& state)
{
    size_t size = state.range(0);
    
    uint64_t *arr = (uint64_t*)std::aligned_alloc(8, size * sizeof(uint64_t));
    benchmark::DoNotOptimize(arr);
    fill_array(arr, size);

    for(auto _: state)
    {
        bool result = std_sort(arr, arr + size);
        benchmark::DoNotOptimize(result);
    }
    benchmark::ClobberMemory();
}

int compare_ints(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;

    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
}

static bool c_qsort(uint64_t* arr, size_t size)
{
    std::qsort(arr, size, sizeof(uint64_t), compare_ints);
    return true;
}

static void c_qsort_function(benchmark::State& state)
{
    size_t size = state.range(0);
    
    uint64_t *arr = (uint64_t*)std::aligned_alloc(8, size * sizeof(uint64_t));
    benchmark::DoNotOptimize(arr);
    fill_array(arr, size);

    for(auto _: state)
    {
        bool result = c_qsort(arr, size);
        benchmark::DoNotOptimize(result);
    }
    benchmark::ClobberMemory();
}

// Register the new benchmark
BENCHMARK(avx512_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<5)
    ->Arg(1<<10)
    ->Arg(1<<12);

BENCHMARK(avx256_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<5)
    ->Arg(1<<10)
    ->Arg(1<<12);

BENCHMARK(normal_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<5)
    ->Arg(1<<10)
    ->Arg(1<<12);

BENCHMARK(std_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<5)
    ->Arg(1<<10)
    ->Arg(1<<12);

BENCHMARK(c_qsort_function)
    ->Unit(benchmark::kNanosecond)
    ->Iterations(iterations)
    ->Arg(1<<5)
    ->Arg(1<<10)
    ->Arg(1<<12);

BENCHMARK_MAIN();
