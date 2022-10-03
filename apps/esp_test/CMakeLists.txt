add_executable(esp_test esp_test.cpp)
target_link_libraries(esp_test PRIVATE asio mme::motion)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simple_test PROPERTY CXX_STANDARD 20)
endif()