#include <algorithm>
#include <ctime>
#include <thread>
#include <iostream>
#include <random>

#include "yijinjing/journal/JournalReader.h"
#include "yijinjing/journal/JournalWriter.h"
#include "yijinjing/journal/Timer.h"
#include "yijinjing/journal/PageCommStruct.h"
#include "yijinjing/journal/FrameHeader.h"
#include "yijinjing/journal/PageHeader.h"
#include "yijinjing/journal/PageSocketStruct.h"

constexpr uint64_t kLoopTimes = 10000000UL;

yijinjing::JournalWriterPtr writer;
yijinjing::JournalReaderPtr reader;

struct TestData
{
  double last_price;
  double open_price;
  double highest_price;
  double lowest_price;
  int64_t local_timestamp_us;
};

void WriteThread()
{
  // 创建一个随机数生成器
  std::random_device rd;
  std::mt19937 gen(rd());

  // 生成介于1和6之间的随机整数，10次
  std::uniform_real_distribution<> distrib(50, 100);

  TestData tick{};
  for (uint64_t i = 0; i < kLoopTimes; ++i)
  {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    tick.last_price = distrib(gen);
    tick.open_price = distrib(gen);
    tick.highest_price = distrib(gen);
    tick.lowest_price = distrib(gen);
    tick.local_timestamp_us = ts.tv_nsec + ts.tv_sec * 1000000000UL;
    // tick.local_timestamp_us = 666;
    writer->write_data(tick, 0, 0);
  }
}

void ReadThread()
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
              << " nano_time: " << frame->getNano()
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
  std::cout << "[PageCommMsg SIZE:]" << sizeof(yijinjing::PageCommMsg) << std::endl;
  std::cout << "[FrameHeader SIZE:]" << sizeof(yijinjing::FrameHeader) << std::endl;
  std::cout << "[PageHeader SIZE:]" << sizeof(yijinjing::PageHeader) << std::endl;
  std::cout << "[PagedSocketRequest SIZE:]" << sizeof(yijinjing::PagedSocketRequest) << std::endl;
  std::cout << "[PagedSocketResponse SIZE:]" << sizeof(yijinjing::PagedSocketResponse) << std::endl;
  std::cout << "[TestData SIZE:]" << sizeof(TestData) << std::endl;
  std::cout << "[short SIZE:]" << sizeof(short) << std::endl;

  writer = yijinjing::JournalWriter::create("./mmap", "flash_store", "writer");
  reader = yijinjing::JournalReader::create("./mmap", "flash_store", yijinjing::getNanoTime(), "reader");

  std::thread rd_thread(ReadThread);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::thread wr_thread(WriteThread);

  wr_thread.join();
  rd_thread.join();
}