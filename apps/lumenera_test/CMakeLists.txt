add_executable(lumenera_test lumenera_test.cpp)

target_link_libraries(lumenera_test PRIVATE mme::lumenera)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simple_test PROPERTY CXX_STANDARD 20)
endif()