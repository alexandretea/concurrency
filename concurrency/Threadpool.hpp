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
            for (size_t i = 0; i < nb; ++i) {
                _workers.emplace_back([this]() {
                    while (true) {
                        // change to condvar with done bool
                        try {
                            task_t  task = _tasks.pop();

                            task();
                        } catch (std::runtime_error const& e) {
                            exit(EXIT_SUCCESS);
                        }
                    }
                });
            }
        }

        ~Threadpool()
        {
            while (not _tasks.empty()) {
            }
            _tasks.stop();
            _done = true;
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
            using return_type = typename std::result_of<Callable(Types...)>::type;

            std::shared_ptr<std::promise<return_type>>  promise(new std::promise<return_type>());
            task_t                              task = [promise, f, args...]() {
                promise->set_value(f(args...));
            };

            {
                std::lock_guard<std::mutex> lock(_mutex);

                _tasks.push(task);
            }
            return promise->get_future();
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
