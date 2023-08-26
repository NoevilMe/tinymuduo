#ifndef A1DA7A24_65FA_4FF5_983D_6D9BEBEC8515
#define A1DA7A24_65FA_4FF5_983D_6D9BEBEC8515

#include <cstring>

namespace muduo {

inline void mem_zero(void *p, size_t n) { ::memset(p, 0, n); }

} // namespace muduo
#endif /* A1DA7A24_65FA_4FF5_983D_6D9BEBEC8515 */
