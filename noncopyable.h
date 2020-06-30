#ifndef __NONCOPYABLE_H_
#define __NONCOPYABLE_H_

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

#endif /* __NONCOPYABLE_H_ */
