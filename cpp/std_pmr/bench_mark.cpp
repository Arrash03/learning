#include <benchmark/benchmark.h>
#include <string>
#include <cstring>
#include <functional>
#include <vector>
// #include <unordered_map>
// #include <utility>
#include <memory_resource>

void replace(std::string& s, const char* from, const char* to)
{
  std::size_t pos = 0;

  while ((pos = s.find(from, pos)) != std::string::npos)
    {
      s.replace(pos, std::strlen(from), to);
      pos += std::strlen(to);
    }
}

void replace_search(std::string& s,const std::boyer_moore_horspool_searcher< const char*>& bms,size_t from, std::string_view to)
{
  auto it = s.begin();
  while (it != s.end())
    {
      it = std::search(it,s.end(),bms);
      if(it != s.end())
        s.replace(it - s.begin(), from, to);
    }
}

static void StringReplaceBMH(benchmark::State& state) {
  //char buffer[512] = {}; // a small buffer on the stack
  //std::fill_n(std::begin(buffer), std::size(buffer) - 1, '_');
  //std::pmr::monotonic_buffer_resource pool{std::data(buffer),std::size(buffer)};
  std::string s{"Hello ${name}, test text is ${test}, ${num},${num},more text"},
  a{"Hello ${name}, more text is ${test}, ${num},${num},more text"},b{"Hello ${name}, more text is long very long, long long text ${test}, ${num},${num},more text"},
  p{"Hello ${name}, test text is long ${num} text ${test}, ${num},more text"};
  std::vector<std::string> v{};
  v.push_back(a);v.push_back(s);v.push_back(p);v.push_back(b);
  size_t i =0;
  //std::string s();
  // Code inside this loop is measured repeatedly
  std::string_view name("${name}"),test("${test}"),num("${num}");
  std::boyer_moore_horspool_searcher bmsname(name.begin(),name.end());
 std::boyer_moore_horspool_searcher bmstest(test.begin(),test.end());
 std::boyer_moore_horspool_searcher bmsnum(num.begin(),num.end());
  for (auto _ : state) {
    std::string copy= v[i%v.size()];
    replace_search(copy, bmsname,name.size(), "Michael");
    replace_search(copy, bmstest,test.size(), "pass");
    replace_search(copy, bmsnum,num.size(), "42");
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(copy);
    benchmark::DoNotOptimize(name);
    benchmark::DoNotOptimize(bmsname);
    benchmark::DoNotOptimize(test);
    benchmark::DoNotOptimize(bmstest);
    benchmark::DoNotOptimize(num);
    benchmark::DoNotOptimize(bmsnum);
    copy.clear();
    i++;
  }
}
// Register the function as a benchmark
BENCHMARK(StringReplaceBMH);//*/

void replace_search(std::pmr::string& s,const std::boyer_moore_horspool_searcher< const char*>& bms,size_t from, std::string_view to)
{
  auto it = s.begin();
  while (it != s.end())
    {
      it = std::search(it,s.end(),bms);
      if(it != s.end())
        s.replace(it - s.begin(), from, to);
    }
}

static void StringReplaceBMHPMR(benchmark::State& state) {
  //char buffer[2048] = {}; // a small buffer on the stack
  std::pmr::monotonic_buffer_resource res;//{std::data(buffer),std::size(buffer)};
  std::pmr::unsynchronized_pool_resource pool{&res};
  std::pmr::string s{"Hello ${name}, test text is ${test}, ${num},${num},more text",&pool},
  a{"Hello ${name}, more text is ${test}, ${num},${num},more text",&pool},b{"Hello ${name}, more text is long very long, long long text ${test}, ${num},${num},more text",&pool},
  p{"Hello ${name}, test text is long ${num} text ${test}, ${num},more text",&pool};
  std::pmr::vector<std::pmr::string> v{&pool};
  v.push_back(a);v.push_back(s);v.push_back(p);v.push_back(b);
  size_t i =0;
  // Code inside this loop is measured repeatedly
  std::string_view name("${name}"),test("${test}"),num("${num}");
  std::boyer_moore_horspool_searcher bmsname(name.begin(),name.end());
 std::boyer_moore_horspool_searcher bmstest(test.begin(),test.end());
 std::boyer_moore_horspool_searcher bmsnum(num.begin(),num.end());
  for (auto _ : state) {
    std::pmr::string copy = v[i%v.size()];
    replace_search(copy, bmsname,name.size(), "Michael");
    replace_search(copy, bmstest,test.size(), "pass");
    replace_search(copy, bmsnum,num.size(), "42");
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(copy);
    benchmark::DoNotOptimize(name);
    benchmark::DoNotOptimize(bmsname);
    benchmark::DoNotOptimize(test);
    benchmark::DoNotOptimize(bmstest);
    benchmark::DoNotOptimize(num);
    benchmark::DoNotOptimize(bmsnum);
    copy.clear();
    i++;
  }
}
// Register the function as a benchmark
BENCHMARK(StringReplaceBMHPMR);//*/

BENCHMARK_MAIN();