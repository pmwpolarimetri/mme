add_executable(image_acquisition_app image_acquisition_app.cpp)

target_link_libraries(image_acquisition_app PRIVATE mme::lumenera libnpy mme::imaging asio mme::motion mme::fwxc lumenera_sdk)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simple_test PROPERTY CXX_STANDARD 20)
endif()