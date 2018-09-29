#include <java/hash.h>
#include <type_traits>
#include <stdlib.h> // For arc4random, if it exists.
#include <chrono>
#include <functional>

namespace java {
// Compile time detection if arc4random_buf is present.
namespace {

// Default case for arc4random detection:
// there is no arc4random.
template<typename Buf, typename Size, typename T = void>
struct has_arc4random_buf_
: std::false_type
{};

// Specialized case for arc4random detection:
// this applies by attempting to find arc4random_buf using ADL.
// Because Buf and Size are template arguments, SFINAE applies
// and the lack of arc4random_buf() is not an error.
template<typename Buf, typename Size>
struct has_arc4random_buf_<Buf, Size, std::void_t<decltype(arc4random(std::declval<Buf>(), std::declval<Size>()))>>
: std::true_type
{};

using has_arc4random_buf = has_arc4random_buf_<void*, std::size_t>::type;
constexpr bool has_arc4random_buf_v = has_arc4random_buf::value;

} /* namespace java::<unnamed> */

auto __hash_nonce() noexcept
-> std::size_t {
  if constexpr(has_arc4random_buf_v) {
    // Use arc4random to create a nonce.
    std::size_t result;
    arc4random_buf(&result, sizeof(result));
    return result;
  } else {
    // Grab a number from the high precision timer, hash that,
    // and hope it's sufficiently hard to guess as to be safe-ish.
    using duration_rep = std::chrono::high_resolution_clock::duration::rep;
    const duration_rep d = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    return std::hash<duration_rep>()(d);
  }
}

} /* namespace java */
