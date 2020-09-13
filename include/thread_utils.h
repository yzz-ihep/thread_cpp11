#ifndef __THREAD_UTILS_H_
#define __THREAD_UTILS_H_

#include <cstring>
#include <thread>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief stdTidToPthreadId std::thread::id转换为pthread_t
 *
 * @param tid std::thread::id
 *
 * @return pthread_t
 */
static pthread_t stdTidToPthreadId(std::thread::id tid) {
    static_assert(
        std::is_same<pthread_t, std::thread::native_handle_type>::value,
        "This assumes that the native handle type is pthread_t");
    static_assert(
        sizeof(std::thread::native_handle_type) == sizeof(std::thread::id),
        "This assumes std::thread::id is a thin wrapper around "
        "std::thread::native_handle_type, but that doesn't appear to be true.");
    // In most implementations, std::thread::id is a thin wrapper around
    // std::thread::native_handle_type, which means we can do unsafe things to
    // extract it.
    pthread_t id;
    std::memcpy(&id, &tid, sizeof(id));
    return id;
}

/**
 * @brief getThreadName 得到线程名称
 *
 * @param id std::thread::id
 *
 * @return 当前线程名称
 */
static std::string getThreadName(std::thread::id id) {
    std::array<char, 16> buf;
    if (id != std::thread::id() &&
            pthread_getname_np(stdTidToPthreadId(id), buf.data(), buf.size()) == 0) {
        return std::string(buf.data());
    } else {
        return "";
    }
}

/**
 * @brief getCurrentThreadName 得到当前线程名称
 *
 * @return std::string ThreadName
 */
static std::string getCurrentThreadName() {
    return getThreadName(std::this_thread::get_id());
}

/**
 * @brief setTidThreadName 设置线程名称
 *
 * @param tid std::thread::id
 * @param name 线程名称
 *
 * @return true - 成功
 */
static bool setTidThreadName(std::thread::id tid, const std::string& name) {
    auto str = name.substr(0, 15);
    char buf[16] = {};
    std::memcpy(buf, str.data(), str.size());
    auto id = stdTidToPthreadId(tid);
    return 0 == pthread_setname_np(id, buf);
}

/**
 * @brief setPidThreadName 设置线程名称
 *
 * @param tid std::thread::id
 * @param name 线程名称
 *
 * @return true - 成功
 */
static bool setPidThreadName(pthread_t pid, const std::string& name) {
    static_assert(
        std::is_same<pthread_t, std::thread::native_handle_type>::value,
        "This assumes that the native handle type is pthread_t");
    static_assert(
        sizeof(std::thread::native_handle_type) == sizeof(std::thread::id),
        "This assumes std::thread::id is a thin wrapper around "
        "std::thread::native_handle_type, but that doesn't appear to be true.");
    // In most implementations, std::thread::id is a thin wrapper around
    // std::thread::native_handle_type, which means we can do unsafe things to
    // extract it.
    std::thread::id id;
    std::memcpy(static_cast<void*>(&id), &pid, sizeof(id));
    return setTidThreadName(id, name);
}

/**
 * @brief setCurrentThreadName 设置当前线程名称
 *
 * @param name 线程名称
 *
 * @return true - 成功
 */
static bool setCurrentThreadName(const std::string& name) {
    return setTidThreadName(std::this_thread::get_id(), name);
}

#endif
