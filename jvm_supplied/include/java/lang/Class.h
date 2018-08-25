#ifndef JAVA_LANG_CLASS_H
#define JAVA_LANG_CLASS_H

#include <memory>
#include <string>
#include <string_view>
#include <cycle_ptr/cycle_ptr.h>
#include <java/serialization/type_def.h>
#include <java/serialization/encdec.h>
#include <java/serialization/data.h>

namespace java::lang {


// Serializable
template <typename T>
class Class;


} /* namespace java::lang */

#include <java/io/Serializable.h>
#include <java/lang/Object.h>
#include <java/lang/reflect/AnnotatedElement.h>
#include <java/lang/reflect/GenericDeclaration.h>
#include <java/lang/reflect/Type.h>

namespace java::serialization {


template<>
class data<java::lang::Object>
: public object_intf
{
public:
  ~data() noexcept override = default;

private:
  /**
   * \brief Test method using predicate matching
   */
  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override;
};

template<>
class tmpl_data<java::lang::Class> final
: public data<java::lang::Object>
#if 0
  ,
  public virtual data<java::io::Serializable>,
  public virtual data<java::lang::reflect::GenericDeclaration>,
  public virtual data<java::lang::reflect::Type>,
  public virtual data<java::lang::reflect::AnnotatedElement>
#endif
{
public:
  ~tmpl_data() noexcept override = default;

  auto getName() const -> std::string {
    return "foobar";
  }

private:
  /**
   * \brief Test method using predicate matching
   */
  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override;
};


/**
 * \brief Serialization definition for java.lang.Class
 * \details Provides translation from runtime type to the types used
 * by the encoder and decoder of a serialized java stream.
 */
template<>
struct template_type_def<java::lang::Class> {

  /**
   * \brief Name of the java class described by this implementation.
   * \returns "java.lang.Class"
   */
  static constexpr auto java_class_name()
  -> std::u16string_view;

private:
  /**
   * \brief Build class description for this type.
   * \returns The \ref java::serialization::stream::new_class_desc__class_desc "class description"
   * for java.lang.Class
   */
  static auto get_class_()
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_class_desc__class_desc>;
public:
  /**
   * \brief Retrieve class description for this type.
   * \returns The \ref java::serialization::stream::new_class_desc__class_desc "class description"
   * for java.lang.Class
   */
  static auto get_class()
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc>;

  /**
   * \brief Encode java.lang.Class
   * \details Glue between C++ type \ref java::lang::Class and the serialization code.
   * \param[in] v The value to encode.
   * \returns Encoded form.
   */
  template<typename T>
  static auto encode(
      const java::lang::Class<T>& v)
  -> cycle_ptr::cycle_gptr<java::serialization::stream::new_object>;

  /**
   * \brief Encode java.lang.Class
   * \details Glue between C++ type \ref java::lang::Class and the serialization code.
   * \param[in] v The value to encode.
   * \param[out] result Encoded type.
   * \param[in,out] handler Context that tracks shared references and breaks recursion.
   * \returns Encoded form.
   */
  template<typename T>
  static auto encode(
      const java::lang::Class<T>& v,
      cycle_ptr::cycle_gptr<java::serialization::stream::new_object> result,
      cycle_handler& handler)
  -> void;

  /**
   * \brief Encodes fields and super type for java.lang.Class
   * \param[in,out] handler Context that tracks shared references and breaks recursion.
   * \param[out] result Encoded type.
   * \param[in] v The value to encode.
   */
  template<typename T>
  static auto encode_contents(
      cycle_handler& handler,
      java::serialization::stream::new_object& result,
      const java::lang::Class<T>& v)
  -> void;
};


} /* namespace java::serialization */

namespace java::lang {


/**
 * \brief C++ type for java.lang.Class
 * \note Auto-generated
 */
template <typename T>
class Class final
: public Object,
  public virtual java::io::Serializable,
  public virtual java::lang::reflect::GenericDeclaration,
  public virtual java::lang::reflect::Type,
  public virtual java::lang::reflect::AnnotatedElement
{
public:
  ~Class() noexcept override;


  virtual auto getName() const
  -> std::string;

private:
  auto do_encode_(java::io::Serializable::tag t) const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> override;

  auto do_encode_(java::io::Serializable::tag t, java::serialization::cycle_handler& h) const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> override;

  /**
   * \brief Test method using predicate matching
   */
  auto __get_class__() const
  -> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> override;
};

template <typename T>
Class<T>::~Class() noexcept = default;

template<typename T>
auto Class<T>::getName() const
-> std::string {
  using conversion_type = std::wstring_convert<
      std::codecvt_utf8_utf16<char16_t>,
      char16_t>;

  // First template argument "T" is the class implementation type.
  // We use its resolver to figure out the java class name.
  const auto& jcn = java::serialization::type_def<T>::java_class_name();

  // Apply UTF-16 to UTF-8 conversion.
  conversion_type conversion;
  return conversion.to_bytes(begin(jcn), end(jcn));
}

template<typename T>
auto Class<T>::do_encode_(java::io::Serializable::tag t) const
-> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> {
  // Don't call super-type method: type_def for java::lang::Class handles inheritance.
  return java::serialization::type_def<java::lang::Class<T>>::encode(*this);
}

template<typename T>
auto Class<T>::do_encode_(java::io::Serializable::tag t, java::serialization::cycle_handler& h) const
-> cycle_ptr::cycle_gptr<const java::serialization::stream::stream_element> {
  // Don't call super-type method: type_def for java::lang::Class handles inheritance.
  return h.encode_field<java::lang::Class<T>>(this->shared_from_this(this));
}

template<typename T>
auto Class<T>::__get_class__() const
-> cycle_ptr::cycle_gptr<const java::serialization::stream::new_class_desc__class_desc> {
  return java::serialization::type_def<java::lang::Class<T>>::get_class();
}



} /* namespace java::lang */

namespace java::serialization {


constexpr auto template_type_def<java::lang::Class>::java_class_name()
-> std::u16string_view {
  using namespace std::string_view_literals;
  return u"java.lang.Class"sv;
}
template<typename T>
auto template_type_def<java::lang::Class>::encode(
      const java::lang::Class<T>& v)
-> cycle_ptr::cycle_gptr<java::serialization::stream::new_object> {
  using namespace java::serialization::stream;

  cycle_handler handler;
  cycle_ptr::cycle_gptr<new_object> result = cycle_ptr::make_cycle<new_object>();
  encode(v, result, handler);
  return result;
}

template<typename T>
auto template_type_def<java::lang::Class>::encode(
      const java::lang::Class<T>& v,
      cycle_ptr::cycle_gptr<java::serialization::stream::new_object> result,
      cycle_handler& handler)
-> void {
  using namespace std::string_view_literals;
  using namespace std::string_literals;
  using namespace java::serialization::stream;

  result->cls = get_class();

  encode_contents(handler, *result, v);
}

template<typename T>
auto template_type_def<java::lang::Class>::encode_contents(
    cycle_handler& handler,
    java::serialization::stream::new_object& result,
    const java::lang::Class<T>& v)
-> void {
  using std::begin;
  using std::end;
  using namespace std::string_view_literals;
  using namespace java::serialization::stream;

  // No local fields.
  result.data.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(get_class()),
      std::forward_as_tuple(
          std::initializer_list<class_desc::field_map::value_type>{
          }));

  // Super type java.lang.Object is serializable
}


} /* namespace java::serialization */


#endif /* JAVA_LANG_CLASS_H */
