#include <atomic>
#include <cstdint>
#include <jthread.hpp>

#include <beyond/core/concurrency/task_queue.hpp>

namespace beyond {

/**
 * @addtogroup core
 * @{
 * @addtogroup concurrency
 * @{
 */

class TaskSystem {
public:
  TaskSystem() = default;
  ~TaskSystem() = default;

  TaskSystem(const TaskSystem&) = delete;
  auto operator=(const TaskSystem&) & -> TaskSystem& = delete;
  TaskSystem(TaskSystem&&) noexcept = delete;
  auto operator=(TaskSystem&&) & noexcept -> TaskSystem = delete;

private:
  const std::uint32_t size_ = nostd::jthread::hardware_concurrency();
};

/** @}@} */

} // namespace beyond

#include <catch2/catch.hpp>

TEST_CASE("Task System", "[beyond.core.concurrency.task_system]")
{
  beyond::TaskSystem task_system;
}
