cmake_minimum_required(VERSION 3.14)
project(ApproximateQueryEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------------------------
# MSVC Runtime Library Fix
# This block forces the use of the static runtime library (/MT or /MTd)
# to prevent linker errors on Windows with Visual Studio.
#--------------------------------------------------------------------
if(MSVC)
  foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                   CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

#--------------------------------------------------------------------
# Main Executable
#--------------------------------------------------------------------
add_executable(aqe src/main.cpp)
target_include_directories(aqe PUBLIC ${PROJECT_SOURCE_DIR}/src)

#--------------------------------------------------------------------
# Testing with Google Test
#--------------------------------------------------------------------
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

# Helper function to easily create a new test executable
function(add_aqe_test test_name)
    add_executable(${test_name} tests/${test_name}.cpp)
    target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${test_name} PRIVATE gtest_main)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add all of your test files
add_aqe_test(core_tests)
add_aqe_test(query_tests)
add_aqe_test(utils_tests)