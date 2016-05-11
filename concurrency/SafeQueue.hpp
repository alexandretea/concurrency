#ifndef SAFEQUEUE_HPP_
# define SAFEQUEUE_HPP_

#include <stdexcept>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <thread>

namespace tea {
namespace concurrency {

    class UserAbort : public std::exception
    {
    public:
        UserAbort() noexcept {}
        virtual ~UserAbort() noexcept {}
    };

    template <typename T>
    class SafeQueue
    {
    public:
        SafeQueue() : _abort(false) {}
        ~SafeQueue() {}

    public:
        SafeQueue(SafeQueue const& other) = delete;
        SafeQueue&      operator=(SafeQueue const& other) = delete;

    public:
        bool
        empty() const
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.empty();
        }

        size_t
        size() const
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.size();
        }

        /*
        ** waits until an element can't be popped or abort is called
        */
        T
        pop()
        {
            std::unique_lock<std::mutex>    lock(_mutex);

            // wait with timeout to handle abort() called just after wait
            while (not _cv.wait_for(lock, std::chrono::milliseconds(200),
                   [this]()->bool { return not _queue.empty() or _abort; })) {
                check_abort();
            }
            check_abort();

            T   v(_queue.front());

            _queue.pop();
            _cv.notify_all();
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


        T const&
        front() const
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.front();
        }

        T const&
        back() const
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

        /*
        ** cancels every waiting operations (example: pop())
        */
        void
        abort()
        {
            _abort = true;
            _cv.notify_all();
        }

        /*
        ** allows waiting operations
        */
        void
        resume()
        {
            _abort = false;
        }

    protected:
        void
        check_abort() const
        {
            if (_abort) {
                throw UserAbort();
            }
        }

    protected:
        std::mutex              _mutex;
        std::condition_variable _cv;
        std::queue<T>           _queue;
        bool                    _abort;
    };
}
}

#endif /* end of include guard: SAFEQUEUE_HPP_ */
