#pragma once

#include<mutex>
#include<queue>
#include<condition_variable>

namespace Utils
{
    namespace Threading
    {
        // Thread safe queue implementation taken from http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
        template<typename T>
        class ThreadSafeQueue
        {
        public:
            ThreadSafeQueue() = default;
            ~ThreadSafeQueue()
            {
                Invalidate();
            }

            // Tries to return the first element in the queue
            // Returns true if successfull or false if failed
            bool TryPop(T& out)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_queue.empty || !m_valid)
                {
                    return false;
                }

                out = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }

            /*
                Returns the first value in the queue.
                Will block until an item is ready unless the queue has been cleared or destroyed.
                Returns true if successfull or false otherwise
            */
            bool WaitPop(T& out)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [&]()
                {
                    return !m_queue.empty() || !m_valid;
                });

                if (!m_valid)
                {
                    return false;
                }

                out = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }

            // Push a new item into the queue.
            void Push(T value)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(std::move(value));
                m_condition.notify_one();
            }

            // Checks whether the queue is empty or not
            bool Empty() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_queue.empty();
            }

            void Clear()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                while (!m_queue.empty())
                {
                    m_queue.pop();
                }
                m_condition.notify_all();
            }

            /*
                Invalidates the queue. Ensures no conditions are being waited on in WaitPop 
                so the application or thread can exit.
                The queue is invalid after this call and any further actions performed on it
                will produce an error.
            */
            void Invalidate()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_valid = false;
                m_condition.notify_all();

                // Not sure if we should clear or not.
                // Potential chance that a reference or something could be held by the queue.

                // Clear();
            }

            // Returns whether or not the queue is valid
            bool IsValid() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_valid;
            }

        private:
            bool m_valid = true;
            std::mutex m_mutex;
            std::queue<T> m_queue;
            std::condition_variable m_condition;
        };
    }
}