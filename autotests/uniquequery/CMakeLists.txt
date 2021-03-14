include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_BINARY_DIR}/lib
)

set(test_SRCS
    test_uniquequery.cpp
)
add_executable(test_uniquequery ${test_SRCS})
target_link_libraries(test_uniquequery
PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::Test
    kjournald
)
add_test(uniquequery test_uniquequery)
