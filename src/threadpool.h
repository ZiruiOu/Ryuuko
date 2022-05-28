#ifndef RYUUKO_SRC_THREADPOOL_H
#define RYUUKO_SRC_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

class ThreadPool;
using ThreadPoolPtr = std::unique_ptr<ThreadPool>;

class ThreadPool final {
 public:
  using task_t = std::function<void()>;
  using worker_id_t = size_t;

 public:
  explicit ThreadPool(size_t num_thread)
      : shut_down_(false), num_thread_(num_thread) {}
  ~ThreadPool() = default;

  // no copy
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // no move
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void start() {
    for (size_t i = 0; i < num_thread_; i++) {
      workers_.emplace_back(&ThreadPool::workerThreadLoop, this);
    }
  }

  void stop() {
    {
      std::scoped_lock latch(mutex_);
      shut_down_ = true;
      worker_cv_.notify_all();
    }
    for (auto&& worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  template <typename FuncType, typename... ArgTypes>
  bool submit(FuncType&& function, ArgTypes&&... arguments) {
    {
      std::scoped_lock<std::mutex> latch(mutex_);
      auto function_wrapper = std::bind(std::forward<FuncType>(function),
                                        std::forward<ArgTypes>(arguments)...);
      task_queue_.push([func = std::move(function_wrapper)] { func(); });
      worker_cv_.notify_one();
    }
    return true;
  }

 private:
  void workerThreadLoop() {
    while (true) {
      task_t task;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        worker_cv_.wait(
            lock, [this]() { return !task_queue_.empty() || shut_down_; });
        if (task_queue_.empty() && shut_down_) {
          return;
        }
        task = std::move(task_queue_.front());
        task_queue_.pop();
      }
      // TODO(ozr) : Use std::future.
      task();
    }
  }

  bool shut_down_;
  size_t num_thread_;
  std::mutex mutex_;
  std::condition_variable worker_cv_;
  std::vector<std::thread> workers_;
  std::queue<task_t> task_queue_;
};

#endif  // ! RYUUKO_THREADPOOL_H