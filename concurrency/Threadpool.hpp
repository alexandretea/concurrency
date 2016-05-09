#ifndef THREADPOOL_HPP_
# define THREADPOOL_HPP_

# include <future>
# include <shared>
# include <functional>
# include <mutex>
# include <vector>
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
            for (size_t i = 0; i < nb; ++i) {
                _workers.emplace_back([this]() {
                    while (true) {
                        // change to condvar with done bool
                        task_t  task = _tasks.pop();

                        task();
                    }
                });
            }
        }

        ~Threadpool()
        {
            _done = true;
            for (auto& worker: _workers) {
                worker.join();
            }
        }

    public:
        Threadpool(Threadpool const& other) = delete;
        Threadpool&     operator=(Threadpool const& other) = delete;

    public:
        template <typename RT, typename ...Types>
        std::future<RT>
        push(std::function<RT (Types&&...)>&& f, Types&&... args)
        {
            std::shared_ptr<std::promise<RT>>   promise = std::make_shared();
            task_t                              task = [promise, f, args...]() {
                promise->set_value(f(args));
            };

            {
                std::lock_guard lock(_mutex);

                _tasks.push(task);
            }
            return promise.get_future();
        }

    protected:
        std::mutex                  _mutex;
        std::vector<std::thread>    _workers;
        SafeQueue<task_t>           _tasks;
        bool                        _done;
    };
}
}

#endif /* end of include guard: THREADPOOL_HPP_ */
