# CMake generated Testfile for 
# Source directory: C:/Users/HP/Desktop/CV PROJ/E6Data
# Build directory: C:/Users/HP/Desktop/CV PROJ/E6Data/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(core_tests "C:/Users/HP/Desktop/CV PROJ/E6Data/build/core_tests.exe")
set_tests_properties(core_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;44;add_test;C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;48;add_aqe_test;C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;0;")
add_test(query_tests "C:/Users/HP/Desktop/CV PROJ/E6Data/build/query_tests.exe")
set_tests_properties(query_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;44;add_test;C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;49;add_aqe_test;C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;0;")
add_test(utils_tests "C:/Users/HP/Desktop/CV PROJ/E6Data/build/utils_tests.exe")
set_tests_properties(utils_tests PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;44;add_test;C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;50;add_aqe_test;C:/Users/HP/Desktop/CV PROJ/E6Data/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
