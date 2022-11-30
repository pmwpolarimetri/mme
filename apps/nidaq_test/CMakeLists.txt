add_executable(nidaq_test "nidaq_test.cpp")

target_link_libraries(nidaq_test PRIVATE mme::nidaq mme::lumenera)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET nidaq_test PROPERTY CXX_STANDARD 20)
endif()