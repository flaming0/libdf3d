#include "ThreadPool.h"

namespace df3d { namespace utils {

struct ThreadPoolWorker
{
    ThreadPool &pool;

    ThreadPoolWorker(ThreadPool &p)
        : pool(p) { }

    void operator()()
    {
        std::function<void ()> job;

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(pool.m_mutex);
                while (!pool.m_stop && pool.m_jobs.empty())
                    pool.m_condition.wait(lock);

                if (pool.m_stop)
                    return;

                job = pool.m_jobs.front();
                pool.m_jobs.pop_front();
            }

            // Do job.
            job();
            // Clean up job.
            auto fn = std::function<void ()>();
            job.swap(fn);
        }
    }
};

ThreadPool::ThreadPool(size_t numWorkers)
    : m_stop(false)
{
    const size_t MAX_WORKERS = 4;

    if (numWorkers > MAX_WORKERS)
        numWorkers = MAX_WORKERS;

    for (size_t i = 0; i < numWorkers; i++)
    {
        m_workers.push_back(std::thread(ThreadPoolWorker(*this)));
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_condition.notify_all();

    for (auto &w : m_workers)
        w.join();
}

void ThreadPool::enqueue(const std::function<void ()> &fn)
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_jobs.push_back(fn);
    }

    m_condition.notify_one();
}

void ThreadPool::clear()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_jobs.clear();
    }

    m_condition.notify_all();
}

} }