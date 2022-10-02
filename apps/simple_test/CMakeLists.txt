add_executable(simple_test simple_test.cpp)

target_link_libraries(simple_test PRIVATE asio mme::imaging mme::motion lumenera_sdk)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simple_test PROPERTY CXX_STANDARD 20)
endif()