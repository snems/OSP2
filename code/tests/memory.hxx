#pragma once
#include <cstddef>

/**
 * @brief Creates an independent copy of the current zone memory state and
 *        returns a unique fork ID. The new fork starts as an exact replica but
 *        evolves separately.
 *
 * @note  Used in tests. Test group may share memory, but tests may be shuffled
 *        and we don't want them to corrupt each other's memory.
 *
 * @return std::size_t
 */
std::size_t ForkMemory();

/**
 * @brief Activates the specified fork, making its memory state the current
 *        working context for subsequent operations.
 */
void SwitchFork(std::size_t id);
