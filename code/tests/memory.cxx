#include "memory.hxx"
#include "../qcommon/qcommon.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>

namespace Memory {

namespace Constants {

static constexpr std::size_t kMemorySize = MAINZONE_STATIC_SIZE;
static constexpr int *kMemory = &mainzone_static[0];

}; // namespace Constants

static std::shared_ptr<int[]> currentFork = nullptr;
static std::vector<std::shared_ptr<int[]>> forks;

}; // namespace Memory

/**
 * @brief Dumps current fork to memory
 */
static void DumpCurrentFork() {
  if (!Memory::currentFork)
    return;

  const int *currentForkBegin = Memory::currentFork.get();
  const int *currentForkEnd = Memory::currentFork.get() +
                              (Memory::Constants::kMemorySize / sizeof(int));

  std::copy(currentForkBegin, currentForkEnd, Memory::Constants::kMemory);
}

/**
 * @brief Loads memory content to current fork
 */
static void LoadToCurrentFork() {
  if (!Memory::currentFork)
    return;

  const int *memoryBegin = Memory::Constants::kMemory;
  const int *memoryEnd = Memory::Constants::kMemory +
                         (Memory::Constants::kMemorySize / sizeof(int));

  std::copy(memoryBegin, memoryEnd, Memory::currentFork.get());
}

/**
 * @brief Allocates and initializes new fork.
 *
 * @return int* - fork
 */
static std::shared_ptr<int[]> ForkFromMainzone() {
  std::shared_ptr<int[]> fork(
      new int[Memory::Constants::kMemorySize / sizeof(int)]);

  std::copy(Memory::Constants::kMemory,
            Memory::Constants::kMemory + (MAINZONE_STATIC_SIZE / sizeof(int)),
            fork.get());

  return fork;
}

std::size_t ForkMemory() {
  // Syncronize previous fork with memory
  LoadToCurrentFork();

  Memory::currentFork = ForkFromMainzone();
  Memory::forks.push_back(Memory::currentFork);

  return Memory::forks.size() - 1;
}

void SwitchFork(std::size_t id) {
  if (Memory::currentFork == Memory::forks.at(id))
    return;

  // Syncronize previous fork with memory
  LoadToCurrentFork();

  Memory::currentFork = Memory::forks[id];

  // Dump newly loaded fork into memory
  DumpCurrentFork();
}
