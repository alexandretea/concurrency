#ifndef SAFEQUEUE_HPP_
# define SAFEQUEUE_HPP_

#include <mutex>

namespace tea {
namespace concurrency {

    template <typename Type>
    class SafeQueue
    {
    public:
        SafeQueue() {}
        ~SafeQueue() {}

    public:
        SafeQueue(SafeQueue const& other) = delete;
        SafeQueue&      operator=(SafeQueue const& other) = delete;

    public:
        bool            empty()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.empty();
        }

        size_t          size()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.size();
        }

        // pop_if ?
        // pop with duration ?
        T&              pop()
        {
            std::lock_guard<std::mutex>   lock(_mutex);
            T&          v = _queue.front();

            _queue.pop();
            return v;
        }

        bool            try_pop(T* value)
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

        void            push(T&& value)
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            _queue.push(value);
        }

        template <typename ...Args>
        void            emplace(Args... args)
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            _queue.emplace(args);
        }

        T&              front()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.front();
        }

        T&              back()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            return _queue.back();
        }

        void            clear()
        {
            std::lock_guard<std::mutex>   lock(_mutex);

            _queue.clear();
        }

    protected:
        std::mutex          _mutex;
        std::queue<Type>    _queue;
    };
}
}

#endif /* end of include guard: SAFEQUEUE_HPP_ */
