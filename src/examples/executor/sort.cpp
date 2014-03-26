#include <experimental/executor>
#include <experimental/future>
#include <experimental/yield>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

using std::experimental::codispatch;
using std::experimental::use_future;
using std::experimental::yield_context;

template <typename Iterator>
void parallel_sort(Iterator begin, Iterator end, yield_context yield)
{
  std::size_t n = end - begin;
  if (n <= 32768)
  {
    std::sort(begin, end);
  }
  else
  {
    copost(
      [=](yield_context yield) { parallel_sort(begin, begin + n / 2, yield); },
      [=](yield_context yield) { parallel_sort(begin + n / 2, end, yield); },
      yield);
    std::inplace_merge(begin, begin + n / 2, end);
  }
}

int main(int argc, char* argv[])
{
  const std::string parallel("parallel");
  const std::string serial("serial");

  if (!(argc == 3 && (argv[1] != parallel || argv[1] != serial)))
  {
    std::cerr << "Usage: `sort parallel <size>' or `sort serial <size>'" << std::endl;
    return 1;
  }

  std::vector<double> vec(std::atoll(argv[2]));
  for (std::size_t i = 0; i < vec.size(); ++i)
    vec[i] = i;

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(vec.begin(), vec.end(), g);

  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  if (argv[1] == parallel)
  {
    dispatch(
      [&](yield_context yield){ parallel_sort(vec.begin(), vec.end(), yield); },
      use_future).get();
  }
  else
  {
    std::sort(vec.begin(), vec.end());
  }

  std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - start;

  std::cout << "sort took ";
  std::cout << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  std::cout << " microseconds" << std::endl;
}
