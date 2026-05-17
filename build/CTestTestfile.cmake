# CMake generated Testfile for 
# Source directory: /Users/adam/courses/Features/Serialization
# Build directory: /Users/adam/courses/Features/Serialization/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(SerializationTests "/Users/adam/courses/Features/Serialization/build/serialization_tests")
set_tests_properties(SerializationTests PROPERTIES  _BACKTRACE_TRIPLES "/Users/adam/courses/Features/Serialization/CMakeLists.txt;39;add_test;/Users/adam/courses/Features/Serialization/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
