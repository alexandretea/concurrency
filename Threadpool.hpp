#ifndef THREADPOOL_HPP_
# define THREADPOOL_HPP_

# include <future>
# include <type_traits>
# include <memory>
# include <functional>
# include <mutex>
# include <vector>
# include <iostream>
# include <queue>
# include "SafeQueue.hpp"

namespace tea {
namespace concurrency {

    class Threadpool
    {
    protected:
        using task_t = std::function<void ()>;

    public:
        Threadpool(size_t nb)
        {
            _workers.reserve(nb);
            for (size_t i = 0; i < nb; ++i) {
                _workers.emplace_back([this]() {
                    while (true) {
                        try { _tasks.pop()(); }
                        catch (UserAbort const&) { break ; }
                    }
                });
            }
        }

        ~Threadpool()
        {
            _tasks.abort();
            for (auto& worker: _workers) {
                worker.join();
            }
        }

    public:
        Threadpool(Threadpool const& other) = delete;
        Threadpool&     operator=(Threadpool const& other) = delete;

    public:
        template <typename Callable, typename ...Types>
        std::future<typename std::result_of<Callable(Types...)>::type>
        push(Callable&& f, Types&&... args)
        {
            using return_type_t =
                typename std::result_of<Callable(Types...)>::type;
            using rt_promise_t = std::promise<return_type_t>;

            std::shared_ptr<rt_promise_t>   promise(new rt_promise_t());
            task_t                          task = [promise, f, args...]() {
                promise->set_value(f(args...));
            };
            {
                std::lock_guard<std::mutex> lock(_mutex);

                _tasks.push(task);
            }
            return promise->get_future();
        }

        size_t
        unsafe_pending_tasks() const
        {
            return _tasks.unsafe_size();
        }

        size_t
        pending_tasks()
        {
            return _tasks.size();
        }

    protected:
        std::mutex                  _mutex;
        std::vector<std::thread>    _workers;
        SafeQueue<task_t>           _tasks;
    };
}
}

#endif /* end of include guard: THREADPOOL_HPP_ */
