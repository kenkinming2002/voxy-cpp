#pragma once

#include <thread_pool.hpp>

template<typename T>
struct Lazy
{
private:
  struct State
  {
    std::atomic<bool>                             done;
    std::aligned_storage_t<sizeof(T), alignof(T)> storage;
  };

public:
  template<typename F>
  Lazy(F f) requires std::is_invocable_r_v<T, F>
  {
    m_state = std::make_shared<State>();
    ThreadPool::instance().enqueue([state=m_state, f=std::move(f)](){
      ::new(&state->storage) T(f());
      state->done.store(true, std::memory_order_release);
    });
  }

  ~Lazy()
  {
    m_state->done.wait(false, std::memory_order_acquire);
  }

public:
  T* try_get()
  {
    if(m_state->done.load(std::memory_order_acquire))
      return std::launder(reinterpret_cast<T*>(&m_state->storage));
    else
      return nullptr;
  }

  T& get()
  {
    m_state->done.wait(false, std::memory_order_acquire);
    return *std::launder(reinterpret_cast<T*>(&m_state->storage));
  }

private:
  std::shared_ptr<State> m_state;
};
