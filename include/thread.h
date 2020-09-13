#ifndef __THREAD_H_
#define __THREAD_H_

#include <atomic>
#include <string>
#include <functional>
#include <exception>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/unistd.h>

#include "thread_utils.h"

class Thread {
    private:
        std::thread _thread;
        std::function<void()> _function;
		std::string _threadName{""};
        pid_t _tid = -1;
        std::atomic_bool _start{false};

    private:
        /**
         * @brief executeFunc 执行函数包装器封装
         */
        void executeFunc() {
            if(_threadName != "") {
                setCurrentThreadName(_threadName);
            }
            else {
                _threadName = getCurrentThreadName();
            }
            _tid = syscall(__NR_gettid);
            try {
                _function();
            } catch(...) {
                throw;
            }
        }

    public:
        Thread() = delete;

        /**
         * @brief Thread 构造函数
         *
         * @param f 要执行的任务
         * @param name 线程名
         */
        Thread(std::function<void()> f): _function(std::move(f)) {}

        /**
         * @brief Thread 构造函数
         *
         * @param f 要执行的任务
         * @param name 线程名
         */
        Thread(const std::string& threadName, std::function<void()> f)
            : _function(std::move(f)), _threadName(threadName) {}

        /**
         * @brief Thread 构造函数
         *
         * @param f 要执行的任务
         * @param name 线程名
         */
        Thread(const std::string& threadName, std::function<void()> f, void* param)
            : _function(std::move(f)), _threadName(threadName) {}

        /**
         * @brief Thread 构造函数
         *
         * @param f 要执行的任务
         * @param name 线程名
         */
        Thread(std::function<void()> f, const std::string& threadName)
            : _function(std::move(f)), _threadName(threadName) {}

        ~Thread() {
            _start.store(false, std::memory_order_relaxed);
        }

        bool setThreadName(const std::string &threadName) {
            _threadName = threadName;
            return setPidThreadName(_tid, threadName);
        };

        std::string getThreadName() {
            return _threadName;
        };

        void start() {
            if (_start.load(std::memory_order_relaxed)) {
                std::cout << "thread has already been started" << "\n";
            } else {
                _start.store(true, std::memory_order_release);
                _thread = std::thread([this]() {
                    executeFunc();
                    _tid = -1;
                });
            }
        }

        void join() {
            std::cout << "Calling Thread::join() on" <<  _threadName << "\n";

            if (!_start.load(std::memory_order_acquire)) {
                std::cout << "Thread::join() called on stopped thread for " << _threadName << "\n";
                return;
            }

            _thread.join();
        }

        /**
         * @brief detach 释放线程,如果线程此时是空闲的,那么线程会退出
         */
        void detach() {
            if (!_start.load(std::memory_order_acquire)) {
                std::cout << "Thread::detach() called on stopped thread for " << _threadName << "\n";
                return;
            }
            _thread.detach();
        }

        /**
         * @brief joinable
         *
         * @return bool true-可以执行join或detach
         */
        bool joinable() {
            return _thread.joinable();
        }
};

#endif
