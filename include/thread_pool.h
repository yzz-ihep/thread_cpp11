#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <mutex>
#include <limits>
#include <memory>
#include <condition_variable>
#include <list>
#include <deque>

#include "thread.h"

class ThreadTask {
    protected:
        std::string _name;
        std::function<void()> _function;

    public:
        ThreadTask() {}

        ThreadTask(ThreadTask* t): _name(t->_name), _function(t->_function) {}

        ThreadTask(std::function<void()> func, const std::string& name = "")
            : _name(name),  _function(std::move(func)) {}

        virtual ~ThreadTask() {}

        virtual void run() {
            _function();
        }

        virtual std::string getTaskName() {
            return _name;
        }

        virtual void setTaskName(const std::string& name) {
            _name = name;
        }
};

class ThreadPool
{
    private:
        size_t _poolSize;
        size_t _taskSize = std::numeric_limits<size_t>::max();
        std::list<std::shared_ptr<Thread>> _threads;
        std::string _poolName;
        std::deque<std::shared_ptr<ThreadTask>> _taskList;
        std::mutex _lock;
        std::condition_variable_any _notEmpty;
        std::condition_variable_any _notFull;
        std::atomic_bool _stop;

        static void runInThread(void *param);

    public:
        ThreadPool(const std::string &poolName, size_t poolSize, size_t taskSize)
            : _poolSize(poolSize), _taskSize(taskSize), _poolName(poolName), _stop(true) {};

        ThreadPool(const std::string &poolName, size_t poolSize)
            : _poolSize(poolSize), _taskSize(poolSize), _poolName(poolName), _stop(true) {};

        ThreadPool(size_t poolSize, size_t taskSize)
            : _poolSize(poolSize), _taskSize(taskSize), _poolName("NacosCliWorkerThread"), _stop(true) {};

        ThreadPool(size_t poolSize)
            : _poolSize(poolSize), _taskSize(poolSize), _poolName("NacosCliWorkerThread"), _stop(true) {};

        std::shared_ptr<ThreadTask> take();
        void put(std::shared_ptr<ThreadTask> t);
        void put(ThreadTask* t);
        void start();
        void stop();
};

#endif
