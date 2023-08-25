#include <thread_pool.hpp>

ThreadPool& ThreadPool::instance()
{
  static ThreadPool thread_pool;
  return thread_pool;
}

ThreadPool::ThreadPool()
{
  unsigned count = std::thread::hardware_concurrency();
  m_threads.reserve(count);
  for(unsigned i=0; i<count; ++i)
    m_threads.emplace_back([this](std::stop_token stoken)
    {
      std::unique_lock lk(m_mutex);
      for(;;)
      {
        m_cv.wait(lk, stoken, [this](){ return !m_tasks.empty(); });
        if(stoken.stop_requested())
          return;

        auto task = std::move(m_tasks.front());
        m_tasks.pop_front();

        lk.unlock();
        task();
        lk.lock();
      }
    });
}

void ThreadPool::enqueue(std::function<void()> task)
{
  std::unique_lock lk(m_mutex);
  m_tasks.push_back(std::move(task));
  lk.unlock();
  m_cv.notify_one();
}

