#ifndef SAFEQUEUE_HPP_
# define SAFEQUEUE_HPP_

#include <condition_variable>
#include <queue>
#include <mutex>

namespace tea {
namespace concurrency {

    template <typename T>
    class SafeQueue
    {
    public:
        SafeQueue() : _stop(false) {}
        ~SafeQueue() {}

    public:
        SafeQueue(SafeQueue const& other) = delete;
        SafeQueue&      operator=(SafeQueue const& other) = delete;

    public:
        bool
        empty()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.empty();
        }

        size_t
        size()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.size();
        }

        T
        pop()
        {
            std::unique_lock<std::mutex>    lock(_mutex);

            _cv.wait(lock, [this]()->bool {
                return not _queue.empty() or _stop;
            });
            if (_stop)
                throw std::runtime_error("user abort"); // switch to our except

            T   v(_queue.front());

            _queue.pop();
            _cv.notify_one();
            return v;
        }

        bool
        try_pop(T* value)
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            if (!_queue.empty())
            {
                *value = _queue.front();
                _queue.pop();
                return true;
            }
            return false;
        }

        void
        push(T const& value)
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            _queue.push(value);
        }

        template <typename ...Args>
        void
        emplace(Args... args)
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            _queue.emplace(args...);
        }

        T&
        front()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.front();
        }

        T&
        back()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.back();
        }

        void
        clear()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            _queue.clear();
        }

        void
        stop()
        {
            _stop = true;
        }

    protected:
        std::mutex              _mutex;
        std::condition_variable _cv;
        std::queue<T>           _queue;
        bool                    _stop;
    };
}
}

#endif /* end of include guard: SAFEQUEUE_HPP_ */
