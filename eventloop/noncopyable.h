#ifndef __MUDUO_NONCOPYABLE_H_
#define __MUDUO_NONCOPYABLE_H_

namespace muduo {
namespace event_loop {

class Noncopyable {
protected:
    Noncopyable(const Noncopyable &) = delete;
    void operator=(const Noncopyable &) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace event_loop
} // namespace muduo

#endif /* __MUDUO_NONCOPYABLE_H_ */
