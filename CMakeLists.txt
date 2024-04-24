cmake_minimum_required(VERSION 3.10)
project (main CXX)

add_executable(test_data test_data.cpp)

set_target_properties(test_data PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED Yes 
)
find_package(Boost REQUIRED COMPONENTS iostreams)
target_link_libraries(test_data PRIVATE Boost::iostreams)
