add_library(ota STATIC
  src/update_manager.cpp
)
target_include_directories(ota PUBLIC include)
target_compile_options(ota PRIVATE -Wall -Wextra -Wpedantic)

add_executable(ota_tests tests/test_ota.cpp)
target_link_libraries(ota_tests PRIVATE ota)
target_compile_options(ota_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME ota_tests COMMAND ota_tests)
