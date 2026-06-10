add_library(bms STATIC
  src/battery_pack.cpp
)
target_include_directories(bms PUBLIC include)
target_compile_options(bms PRIVATE -Wall -Wextra -Wpedantic)

add_executable(bms_tests tests/test_bms.cpp)
target_link_libraries(bms_tests PRIVATE bms)
target_compile_options(bms_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME bms_tests COMMAND bms_tests)
