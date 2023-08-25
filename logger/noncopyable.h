#ifndef B1C1FF97_47FD_4BED_BE5C_419B92166E33
#define B1C1FF97_47FD_4BED_BE5C_419B92166E33

namespace muduo {
namespace log {

class Noncopyable {
protected:
    Noncopyable(const Noncopyable &) = delete;
    void operator=(const Noncopyable &) = delete;

protected:
    Noncopyable() = default;
    ~Noncopyable() = default;
};

} // namespace log
} // namespace muduo

#endif /* B1C1FF97_47FD_4BED_BE5C_419B92166E33 */
