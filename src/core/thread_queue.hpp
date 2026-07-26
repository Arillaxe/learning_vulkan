

#ifndef THREAD_QUEUE_HPP
#define THREAD_QUEUE_HPP

#include <mutex>
#include <queue>
#include <utility>

template <typename T>
class ThreadQueue
{
public:
  void push(T value)
  {
    std::lock_guard lock(m_mutex);
    m_queue.push(std::move(value));
  }

  bool tryPop(T &out)
  {
    std::unique_lock lock(m_mutex, std::try_to_lock);

    if (!lock.owns_lock() || m_queue.empty())
      return false;

    out = std::move(m_queue.front());
    m_queue.pop();

    return true;
  }

private:
  std::mutex m_mutex;
  std::queue<T> m_queue;
};

#endif // THREAD_QUEUE_HPP
