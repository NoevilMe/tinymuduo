#ifndef __MUDUO_NONCOPYABLE_H_
#define __MUDUO_NONCOPYABLE_H_

/**
 * @brief Same file in eventloop
 *
 */

namespace muduo {

class Noncopyable {
protected:
    Noncopyable(const Noncopyable &) = delete;
    void operator=(const Noncopyable &) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace muduo

#endif /* __MUDUO_NONCOPYABLE_H_ */
