#include <catch2/catch.hpp>

#include <beyond/core/concurrency/task_queue.hpp>

#include <array>
#include <string>
#include <vector>

TEST_CASE("Task Queue push and pop", "[beyond.core.concurrency.task_queue]")
{
  std::array<beyond::TaskQueue, 4> queues;
  auto& aq = queues[0];
  auto& bq = queues[1];
  auto& cq = queues[2];
  auto& dq = queues[3];

  std::mutex m;
  std::vector<std::string> output;
  auto strout = [&](std::string str) {
    std::lock_guard<std::mutex> l(m);
    output.emplace_back(std::move(str));
  };

  std::vector<std::thread> threads;

  const auto a1 = "a1                      ( 1)";
  const auto b1 = "    b1                  ( 2)";
  const auto d1 = "            d1          ( 3)";
  const auto c1 = "        c1              ( 4)";
  const auto c2 = "        c2              ( 5)";
  const auto d2 = "            d2          ( 6)";

  aq.push([&]() { strout(a1); });
  bq.push([&]() { strout(b1); });
  dq.push([&]() { strout(d1); });
  cq.push([&]() { strout(c1); });
  cq.push([&]() { strout(c2); });
  dq.push([&]() { strout(d2); });

  SECTION("Pop and run all the tasks")
  {
    for (auto& queue : queues) {
      while (!queue.empty()) {
        threads.emplace_back([&]() {
          auto task = queue.pop();
          REQUIRE(task != std::nullopt);
          return std::move(*task);
        }());
      }
    }

    for (auto& thread : threads) {
      thread.join();
    }

    REQUIRE(output.size() == 6);
  }

  SECTION("Try to pop and run all the tasks")
  {
    for (auto& queue : queues) {
      while (!queue.empty()) {
        threads.emplace_back([&]() {
          auto task = queue.try_pop();
          REQUIRE(task != std::nullopt);
          return std::move(*task);
        }());
      }
    }

    for (auto& thread : threads) {
      thread.join();
    }

    REQUIRE(output.size() == 6);
  }
}
