FIND_PACKAGE(HDF5 REQUIRED CXX)
include_directories(${HDF5_INCLUDE_DIRS})

include_directories(. ../lib/include)

add_executable(testCounters testCounters.cpp tests.cpp)
target_link_libraries(testCounters MCPlusPlus)

add_test(NAME "testCounters" COMMAND testCounters)
set_tests_properties(
    testCounters PROPERTIES PASS_REGULAR_EXPRESSION "testCounters PASSED")

add_executable(testHistogram testHistogram.cpp tests.cpp)
target_link_libraries(testHistogram MCPlusPlus)

add_test(NAME "testHistogram" COMMAND testHistogram)
set_tests_properties(
    testHistogram PROPERTIES PASS_REGULAR_EXPRESSION "testHistogram PASSED")
