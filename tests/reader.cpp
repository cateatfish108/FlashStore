#include <algorithm>
#include <ctime>
#include <thread>
#include <iostream>
#include <random>
#include "yijinjing/journal/JournalReader.h"
#include "yijinjing/journal/JournalWriter.h"
#include "yijinjing/journal/Timer.h"

constexpr uint64_t kLoopTimes = 10000000UL;
yijinjing::JournalReaderPtr reader;

struct TestData
{
  double last_price;
  double open_price;
  double highest_price;
  double lowest_price;
  int64_t local_timestamp_us;
};

void Reader()
{
  std::vector<uint64_t> time_cost;
  yijinjing::FramePtr frame;
  for (uint64_t i = 0; i < kLoopTimes; ++i)
  {
    do
    {
      frame = reader->getNextFrame();
    } while (!frame);
    auto *tick = reinterpret_cast<TestData *>(frame->getData());
    std::cout << "[Tick]"
              << " last_price: " << tick->last_price
              << " open_price: " << tick->open_price
              << " highest_price: " << tick->highest_price
              << " lowest_price: " << tick->lowest_price
              << " local_timestamp_us: " << tick->local_timestamp_us
              << std::endl;
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_cost.emplace_back(ts.tv_nsec + ts.tv_sec * 1000000000UL - tick->local_timestamp_us);
  }

  sort(time_cost.begin(), time_cost.end());

  uint64_t sum = 0;
  for (auto v : time_cost)
  {
    sum += v;
  }
  printf("mean:%lu, min:%lu, 25th:%lu, 50th:%lu 75th:%lu max:%lu\n", sum / kLoopTimes, time_cost[0],
         time_cost[time_cost.size() / 4], time_cost[time_cost.size() / 2],
         time_cost[time_cost.size() * 3 / 4], *time_cost.rbegin());
}

int main()
{
  reader = yijinjing::JournalReader::create("./mmap", "flash_store", yijinjing::getNanoTime(), "reader");
  Reader();
}