add_executable(filterwheel_test filterwheel_test.cpp)
target_link_libraries(filterwheel_test PRIVATE mme::fwxc)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simple_test PROPERTY CXX_STANDARD 20)
endif()