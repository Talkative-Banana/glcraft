#pragma once

#include <cassert>
#include <thread>

inline std::thread::id mainThreadId;

inline void SetMainThread() { mainThreadId = std::this_thread::get_id(); }

inline void AssertMainThread() {
  assert(std::this_thread::get_id() == mainThreadId &&
         "This path must run on the main thread");
}
