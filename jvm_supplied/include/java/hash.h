#ifndef JAVA_HASH_H
#define JAVA_HASH_H

#include <cstddef>
#include <functional>

namespace java {

/**
 * \brief Algorithm used to combine hash codes.
 * \details This is the FNV algorithm, which is a hash function that operates on individual bytes.
 *
 * The documentation of the algorithm can be found on
 * [the FNV web page](http://www.isthe.com/chongo/tech/comp/fnv/).
 *
 * We use the FNV hash primarily to compute a hash code over multiple hash codes.
 */
class fnv_hash {
 public:
  ///\brief Create a new hasher.
  constexpr fnv_hash() noexcept
  : fnv_hash(offset_basis())
  {}

  ///\brief Create a new hasher based on the result of a previous hasher.
  constexpr fnv_hash(std::size_t h) noexcept
  : h_(h)
  {}

  ///\brief Retrieve the hash code result.
  ///\returns The hash code.
  constexpr auto result() const noexcept -> std::size_t {
    return h_;
  }

  ///\brief Add a number of opaque bytes.
  ///\param b Pointer to the start of the memory buffer.
  ///\param n Length of the memory buffer.
  ///\returns *this
  auto add_bytes(const void* b, std::size_t n) noexcept -> fnv_hash& {
    const std::uint8_t* arr = reinterpret_cast<const std::uint8_t*>(b);
    while (n > 0) {
      h_ ^= arr[0];
      h_ *= fnv_prime(); // FNV-1a: multiply after XOR

      ++arr;
      --n;
    }

    return *this;
  }

  ///\brief Add a hash code constant to the state.
  ///\param x The output of a hash function (for example std::hash).
  ///\returns *this
  auto add_hashcode(std::size_t x) noexcept -> fnv_hash& {
    for (unsigned int i = 0; i < sizeof(std::size_t); ++i) {
      const unsigned int shift = (sizeof(std::size_t) - i - 1u) * 8u;
      const unsigned std::uint8_t byte = ((x >> shift) & 0xffu);

      h_ ^= byte;
      h_ *= fnv_prime(); // FNV-1a: multiply after XOR
    }

    return *this;
  }

 private:
  ///\brief The offset basis for FNV hash.
  ///\returns A suitable offset basis, for the size of `std::size_t`.
  static constexpr auto offset_basis() noexcept -> std::size_t {
    if constexpr(sizeof(std::size_t) == 32u) {
      // 2166136261
      return std::size_t(0x811c9dc5ull) << 0;
    } else if constexpr(sizeof(std::size_t) == 64u) {
      // 14695981039346656037
      return std::size_t(0xcbf29ce484222325ull) << 0;
    } else if constexpr(sizeof(std::size_t) == 128u) {
      // 144066263297769815596495629667062367629
      return std::size_t(0x6c62272e07bb0142ull) << 64
          | std::size_t(0x62b821756295c58dull) << 0;
    } else if constexpr(sizeof(std::size_t) == 256u) {
      // 100029257958052580907070968620625704837092796014241193945225284501741471925557
      return std::size_t(0xdd268dbcaac55036ull) << 192
          | std::size_t(0x2d98c384c4e576ccull) << 128
          | std::size_t(0xc8b1536847b6bbb3ull) << 64
          | std::size_t(0x1023b4c8caee0535ull) << 0;
    } else if constexpr(sizeof(std::size_t) == 512u) {
      // 9659303129496669498009435400716310466090418745672637896108374329434462657994582932197716438449813051892206539805784495328239340083876191928701583869517785
      return std::size_t(0xb86db0b1171f4416ull) << 448
          | std::size_t(0xdca1e50f309990acull) << 384
          | std::size_t(0xac87d059c9000000ull) << 320
          | std::size_t(0xd21ull) << 256
          | std::size_t(0xe948f68a34c192f6ull) << 192
          | std::size_t(0x2ea79bc942dbe7ceull) << 128
          | std::size_t(0x182036415f56e34bull) << 64
          | std::size_t(0xac982aac4afe9fd9ull) << 0;
    } else if constexpr(sizeof(std::size_t) == 1024u) {
      // 14197795064947621068722070641403218320880622795441933960878474914617582723252296732303717722150864096521202355549365628174669108571814760471015076148029755969804077320157692458563003215304957150157403644460363550505412711285966361610267868082893823963790439336411086884584107735010676915
      return std::size_t(0x5f7a76758ecc4dull) << 896
          | std::size_t(0x32e56d5a591028b7ull) << 832
          | std::size_t(0x4b29fc4223fdada1ull) << 768
          | std::size_t(0x6c3bf34eda3674daull) << 704
          | std::size_t(0x9a21d90000000000ull) << 640
          | std::size_t(0ull) << 576
          | std::size_t(0ull) << 512
          | std::size_t(0ull) << 448
          | std::size_t(0ull) << 384
          | std::size_t(0ull) << 320
          | std::size_t(0x4c6d7ull) << 256
          | std::size_t(0xeb6e73802734510aull) << 192
          | std::size_t(0x555f256cc005ae55ull) << 128
          | std::size_t(0x6bde8cc9c6a93b21ull) << 64
          | std::size_t(0xaff4b16c71ee90b3ull) << 0;
    }
  }

  ///\brief The prime for FNV hash.
  ///\returns A suitable prime, for the size of `std::size_t`.
  static constexpr auto fnv_prime() noexcept -> std::size_t {
    if constexpr(sizeof(std::size_t) == 32u) {
      // 16777619
      return std::size_t(1) << 24
          + std::size_t(1) << 8
          + 0x93u;
    } else if constexpr(sizeof(std::size_t) == 64u) {
      // 1099511628211
      return std::size_t(1) << 40
          + std::size_t(1) << 8
          + 0xb3u;
    } else if constexpr(sizeof(std::size_t) == 128u) {
      // 309485009821345068724781371
      return std::size_t(1) << 88
          + std::size_t(1) << 8
          + 0x3bu;
    } else if constexpr(sizeof(std::size_t) == 256u) {
      // 374144419156711147060143317175368453031918731002211
      return std::size_t(1) << 168
          + std::size_t(1) << 8
          + 0x63u;
    } else if constexpr(sizeof(std::size_t) == 512u) {
      // 35835915874844867368919076489095108449946327955754392558399825615420669938882575126094039892345713852759
      return std::size_t(1) << 344
          + std::size_t(1) << 8
          + 0x57u;
    } else if constexpr(sizeof(std::size_t) == 1024u) {
      // 5016456510113118655434598811035278955030765345404790744303017523831112055108147451509157692220295382716162651878526895249385292291816524375083746691371804094271873160484737966720260389217684476157468082573
      return std::size_t(1) << 680
          + std::size_t(1) << 8
          + 0x8du;
    }
  }

  ///\brief The current state of the hash.
  std::size_t h_;
};

/**
 * \brief Hash combination object.
 * \details This object accepts fields and produces a hash code based on those fields.
 *
 * \code
 * // Non-overriding case.
 * std::size_t hash_code() const {
 *   return hash_combiner()
 *       << this->field1
 *       << this->field2
 *       << this->field3
 *       << ...
 *       << this->fieldN;
 * }
 *
 * // Overriding case.
 * std::size_t hash_code() const override {
 *   // NOTE: never invoke hash_code on java.lang.Object.
 *   // It's implementation is based on the address of the Object,
 *   // making two different instances always compare unequal.
 *   return hash_combiner(this->my_super_type::hash_code())
 *       << this->field1
 *       << this->field2
 *       << this->field3
 *       << ...
 *       << this->fieldN;
 * }
 * \endcode
 *
 * If the field being added is an std::size_t, it is assumed to be a precomputed hash.
 * This allows for a case where a runtime constant is injected, as protection against
 * hash table attacks.
 * It is advised to implement this kind of protection.
 * \code
 * std::size_t hash_code() const {
 * #if HAS_ARC4RANDOM
 *   // Function-local static constant is initialized at first use and then
 *   // maintains its value, providing cookie behaviour.
 *   // Due to the initialization here, it'll change between each run of the program.
 *   static const std::size_t runtime_constant = arc4random();
 * #endif
 *
 *   return hash_combiner()
 * #if HAS_ARC4RANDOM
 *       << runtime_constant
 * #endif
 *       << field1
 *       << field2
 *       << field3
 *       << ...
 *       << fieldN;
 * }
 * \encode
 *
 * For each field added, the object computes the hash code, using std::hash<T>.
 * It then adds this hash code to the internal state.
 * This effectively treats the hash codes as a sequence of bytes.
 *
 * The current implementation uses the \ref fnv_hash "FNV hash" algorithm.
 */
class hash_combiner {
 public:
  constexpr hash_combiner() noexcept = default;

  constexpr hash_combiner(std::size_t parent_hash_code) noexcept
  : state_(parent_hash_code)
  {}

  /**
   * \brief Permute the hash state based on a field.
   * \details The hash code of the field is computed and then
   * added to the internal hash function.
   * \param v Reference to the field that is to be added.
   * \returns *this
   */
  template<typename T>
  auto operator<<(const T& v) & noexcept -> hash_combiner& {
    if constexpr(std::is_same_v<std::size_t, T>) {
      state_.add_hashcode(v);
    } else {
      using hasher = std::hash<T>;
      state_.add_hashcode(hasher()(v));
    }

    return *this;
  }

  /**
   * \brief Permute the hash state based on a field.
   * \details The hash code of the field is computed and then
   * added to the internal hash function.
   *
   * This specialization exists to allow rvalue computation.
   * \param v Reference to the field that is to be added.
   * \returns *this
   */
  template<typename T>
  auto operator<<(const T& v) && noexcept -> hash_combiner&& {
    return std::move(*this << v);
  }

  /**
   * \brief Acquire the result of the hash operation.
   * \details By using a conversion operator, the hash_combiner can
   * be returned directly by the hash code computation function.
   * \returns Final hash code.
   */
  constexpr operator std::size_t() const noexcept {
    return state_.result();
  }

 private:
  fnv_hash state_;
};

} /* namespace java */

#endif /* JAVA_HASH_H */
