#pragma once

/*
    Thread pool class keeps a set of threads running ready to execute any incoming jobs.
    Taken from http://roar11.com/2016/01/a-platform-independent-thread-pool-using-c14/
*/

#include "ThreadSafeQueue.h"

#include <algorithm>
#include <future>

namespace Utils
{
    namespace Threading
    {
        class ThreadPool
        {
            class IThreadTask
            {
            public:
                IThreadTask() = default;
                virtual ~IThreadTask() = default;
                IThreadTask(const IThreadTask&) = delete;
                IThreadTask& operator=(const IThreadTask&) = delete;
                IThreadTask(IThreadTask&&) = default;
                IThreadTask& operator=(IThreadTask&&) = default;

                virtual void Execute() = 0;
            };// IThreadTask

            template<typename Function>
            class ThreadTask : public IThreadTask
            {
            public:
                ThreadTask(Function& func)
                    :m_function(std::move(func))
                {
                }

                virtual ~ThreadTask() = default;
                ThreadTask(const ThreadTask&) = delete;
                ThreadTask& operator=(const ThreadTask&) = delete;
                ThreadTask(ThreadTask&&) = default;
                ThreadTask& operator=(ThreadTask&&) = default;

                // Run the task
                void Execute() override
                {
                    m_function();
                }

            private:
                Function m_function;
            }; // ThreadTask

        public:
            /*
                A Wrapper around a std::future that will ensure it blocks and waits for execution to finish 
                before going out of scope.
            */
            template <typename T>
            class TaskFuture
            {
            public:
                TaskFuture(std::future<T>&& future)
                    :m_future(std::move(future))
                {
                }

                TaskFuture(const TaskFuture&) = delete;
                TaskFuture& operator=(const TaskFuture&) = delete;
                TaskFuture(TaskFuture&&) = default;
                TaskFuture& operator=(TaskFuture&&) = default;

                ~TaskFuture()
                {
                    if (m_future.valid())
                    {
                        m_future.get();
                    }
                }

                T Get()
                {
                    return m_future.get();
                }

            private:
                std::future<T> m_future;
            }; // TaskFuture

            // Non copyable and non assignable
            ThreadPool(const ThreadPool&) = delete;
            ThreadPool& operator=(const ThreadPool&) = delete;

            ThreadPool()
                :ThreadPool(std::max(std::thread::hardware_concurrency(), 2u) - 1)
            {

            }

            ThreadPool(const uint32_t numThreads)
                :m_done(false),
                m_workQueue(),
                m_threads()
            {
                try
                {
                    for (uint32_t i = 0; i < numThreads; ++i)
                    {
                        m_threads.emplace_back(&ThreadPool::Worker, this);
                    }
                }
                catch (...)
                {
                    Destroy();
                    throw;
                }
            }

            ~ThreadPool()
            {
                Destroy();
            }

            // Submit work to be executed by a thread
            template <typename Function, typename... Args>
            auto Submit(Function&& func, Args&&... args)
            {
                auto boundTask = std::bind(std::forward<Function>(func), std::forward<Args>(args)...);
                using ResultType = std::result_of_t<decltype(boundTask)()>;
                using PackagedTask = std::packaged_task<ResultType()>;
                using TaskType = ThreadTask<PackagedTask>;

                PackagedTask task(std::move(boundTask));
                TaskFuture<ResultType> result(task.get_future());
                m_workQueue.Push(std::make_unique<TaskType>(std::move(task)));
                return result;
            }

        private:
            // Constantly running function each thread uses to acquire and execute a task from the queue.
            void Worker()
            {
                while (!m_done)
                {
                    std::unique_ptr<IThreadTask> task{ nullptr };
                    if (m_workQueue.WaitPop(task))
                    {
                        task->Execute();
                    }
                }
            }

            // Invalidates the queue and joins all running threads
            void Destroy()
            {
                m_done = true;
                m_workQueue.Invalidate();
                for (auto& thread : m_threads)
                {
                    if (thread.joinable())
                    {
                        thread.join();
                    }
                }
            }

        private:
            std::atomic_bool m_done;
            ThreadSafeQueue<std::unique_ptr<IThreadTask>> m_workQueue;
            std::vector<std::thread> m_threads;
        }; // ThreadPool

        namespace DefaultThreadPool
        {
            /*
                Get the default global thread pool for the application
                This pool is created with the max available hardware threads available.
                i.e. (std::thread::hardware_concurrency() - 1)
            */
            inline ThreadPool& GetThreadPool()
            {
                static ThreadPool defaultPool;
                return defaultPool;
            }

            // Submit a job to the default thread pool
            template <typename Function, typename... Args>
            inline auto SubmitJob(Function&& func, Args&&... args)
            {
                return GetThreadPool().Submit(std::forward<Function>(func), std::forward<Args>(args)...);
            }
        }
    }
}