#pragma once

#include <chrono>
#include <mutex>
#include <forward_list>
#include <utility>
#include <algorithm>


namespace common_helpers
{
  template<typename Ty>
  class ForgettableMemory {
    struct ForgettableBlock {
      Ty block;
      std::chrono::high_resolution_clock::time_point due_time;

      template<typename Rep, typename Period, class... Args>
      ForgettableBlock(std::chrono::duration<Rep, Period> duration, Args&&... args)
        : due_time(std::chrono::high_resolution_clock::now() + duration),
          block(  std::forward<Args>(args)... )
      { }
    };

    std::recursive_mutex mtx{};
    std::forward_list<ForgettableBlock> storage{};


  public:
    template<typename Rep, typename Period, class... Args>
    Ty& create(std::chrono::duration<Rep, Period> duration, Args&&... args) {
      std::lock_guard lock(mtx);

      auto& new_ele = storage.emplace_front(duration, std::forward<Args>(args)...);
      return new_ele.block;
    }

    bool is_alive(const Ty& block) {
      std::lock_guard lock(mtx);
      
      auto ele_it = std::find_if(storage.begin(), storage.end(), [&block](const ForgettableBlock &item){
        return &item.block == &block;
      });
      return storage.end() != ele_it;
    }

    void destroy(const Ty& block) {
      std::lock_guard lock(mtx);
      
      storage.remove_if([&block](const ForgettableBlock &item){
        return &item.block == &block;
      });
    }

    void destroy_all() {
      std::lock_guard lock(mtx);
      
      storage.clear();
    }

    void cleanup() {
      std::lock_guard lock(mtx);
      
      const auto now = std::chrono::high_resolution_clock::now();
      storage.remove_if([&now](const ForgettableBlock &item){
        return now > item.due_time;
      });

    }

  };
}
