#include <iostream>

#include "thread_pool.h"
#include "thread.h"

void ThreadPool::runInThread(void *param) {
    ThreadPool* thisobj = static_cast<ThreadPool*>(param);

    while (!thisobj->_stop.load(std::memory_order_acquire))
    {
        auto t = thisobj->take();

        try
        {
            t->run();
        }
        catch (std::exception &e)
        {
            std::cout << "Exception happens when executing:" << "\n";
            std::cout << "Thread pool Name: " << thisobj->_poolName << " Task name: " << t->getTaskName();
            std::cout << "Raison: " << e.what() << "\n";
        }
        catch (...)
        {
            std::cout << "Unknown exception happens when executing: " << "\n";
            std::cout << "Thread pool Name: " << thisobj->_poolName << " Task name: " << t->getTaskName();
            throw;
        }
        std::cout << "Thread finished task:%s " << t->getTaskName() << " without problem" << "\n";
    }
}

std::shared_ptr<ThreadTask> ThreadPool::take() {
    _lock.lock();
    while(_taskList.empty() && !_stop) {
        _notEmpty.wait(_lock);
    }

    if(!_taskList.empty())
    {
        auto curTask = _taskList.front();
        _taskList.pop_front();

        _notFull.notify_all();
        _lock.unlock();
        return curTask;
    }
    _lock.unlock();

    return std::shared_ptr<ThreadTask>(new ThreadTask([]() {}, "dummy for stop"));
};

void ThreadPool::put(std::shared_ptr<ThreadTask> t)
{

    _lock.lock();
    _notFull.wait(_lock, [this]() {
        return _taskList.size() < _poolSize;
    });

    if (!_stop.load(std::memory_order_relaxed))
    {
        _taskList.push_back(t);
        _lock.unlock();
        _notEmpty.notify_all();
    } else {
        _lock.unlock();
        //The thread pool is stopped, we need to run it locally
        std::cout << "Running locally since the threadpool is stopped" << "\n";
        t->run();
    }
};

void ThreadPool::put(ThreadTask* task)
{
    auto t = std::shared_ptr<ThreadTask>(task);
    _lock.lock();
    _notFull.wait(_lock, [this]() {
        return _taskList.size() < _poolSize;
    });

    if (!_stop.load(std::memory_order_relaxed))
    {
        _taskList.push_back(t);
        _lock.unlock();
        _notEmpty.notify_all();
    } else {
        _lock.unlock();
        //The thread pool is stopped, we need to run it locally
        std::cout << "Running locally since the threadpool is stopped" << "\n";
        t->run();
    }
};

void ThreadPool::start()
{
    if (!_stop.load(std::memory_order_relaxed))
    {
        std::cout << "Thread pool named " << _poolName  << " is started multiple times" << "\n";
        return;
    }

    _stop.store(false, std::memory_order_relaxed);
    for (size_t i = 0; i < _poolSize; i++)
    {
        _threads.emplace_back(std::make_shared<Thread>(_poolName + "-poolthread-" + std::to_string(i), std::bind(runInThread, this)));
        _threads.back()->start();
    }
};

void ThreadPool::stop()
{
    if (_stop.load(std::memory_order_relaxed))
    {
        return;
    }

    _stop.store(true, std::memory_order_release);

    _notEmpty.notify_all();
    for(auto t : _threads) {
        t->join();
    }
};
