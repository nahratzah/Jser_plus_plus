include (CTest)
find_package (UnitTest++)

find_package (Java REQUIRED)
include (UseJava)

macro (do_deser_test binary)
  add_jar (${binary}Jar ${binary}.java)
  get_target_property(_${binary}JarFile ${binary}Jar JAR_FILE)

  add_custom_target (${binary}File
    ${Java_JAVA_EXECUTABLE} -cp ${_${binary}JarFile} ${binary} ${binary}.ser
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    BYPRODUCTS ${binary}.ser
    COMMENT "Generating ${binary}.ser")
  add_dependencies (${binary}File ${binary}Jar)

  add_executable (test_${binary} ${binary}.cc)
  target_link_libraries (test_${binary} PRIVATE asio_jmx)
  target_link_libraries (test_${binary} PRIVATE UnitTest++)
  add_test (${binary} test_${binary} ${CMAKE_CURRENT_BINARY_DIR}/${binary}.ser)
  add_dependencies (test_${binary} ${binary}File)
endmacro (do_deser_test)

if (UnitTest++_FOUND)
  include_directories(${UTPP_INCLUDE_DIRST})

  do_deser_test (SerString)
  do_deser_test (SerClass)
  do_deser_test (SerField)
  do_deser_test (SerArray)
  do_deser_test (SerReset)
  do_deser_test (SerRecursive)
  do_deser_test (SerException)
  do_deser_test (SerEnum)
endif()
