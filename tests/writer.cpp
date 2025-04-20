#include <algorithm>
#include <ctime>
#include <thread>
#include "yijinjing/journal/JournalReader.h"
#include "yijinjing/journal/JournalWriter.h"
#include "yijinjing/journal/Timer.h"
#include <iostream>
#include <random>

constexpr uint64_t kLoopTimes = 10000000UL;
yijinjing::JournalWriterPtr writer;

struct TestData
{
  double last_price;
  double open_price;
  double highest_price;
  double lowest_price;
  int64_t local_timestamp_us;
};

void Writer()
{
  // 创建一个随机数生成器
  std::random_device rd;
  std::mt19937 gen(rd());

  // 生成介于1和6之间的随机整数，10次
  std::uniform_real_distribution<> distrib(1, 100);

  TestData tick{};
  //   for (uint64_t i = 0; i < kLoopTimes; ++i)
  while (true)
  {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tick.last_price = distrib(gen);
    tick.open_price = distrib(gen);
    tick.highest_price = distrib(gen);
    tick.lowest_price = distrib(gen);
    tick.local_timestamp_us = ts.tv_nsec + ts.tv_sec * 1000000000UL;
    writer->write_data(tick, 0, 0);
  }
}

int main()
{
  writer = yijinjing::JournalWriter::create("./mmap", "flash_store", "writer");
  Writer();
}