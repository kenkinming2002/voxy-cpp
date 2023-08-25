#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <vector>
#include <deque>

class ThreadPool
{
public:
  static ThreadPool& instance();
  ThreadPool();

public:
  void enqueue(std::function<void()> task);

private:
  std::mutex                        m_mutex;
  std::condition_variable_any       m_cv;
  std::deque<std::function<void()>> m_tasks;

  std::vector<std::jthread> m_threads;
};
