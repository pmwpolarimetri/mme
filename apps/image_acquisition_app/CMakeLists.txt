add_executable(image_acquisition_app image_acquisition_app.cpp)

find_package(PythonLibs REQUIRED)
include_directories("C:\\Users\\PolarimetriD4-112\\Anaconda3\\include")
link_directories("C:\\Users\\PolarimetriD4-112\\Anaconda3\\libs")

set(PYLIB_DIR "C:\\Users\\PolarimetriD4-112\\Anaconda3\\libs")
message(${PYLIB_DIR})
find_library(PYlib python37.lib HINTS ${PYLIB_DIR})
include_directories(${PYLIB_DIR})
message(${PYlib})

target_link_libraries(image_acquisition_app PRIVATE mme::lumenera libnpy mme::imaging asio mme::motion mme::fwxc lumenera_sdk ${PYlib})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET simple_test PROPERTY CXX_STANDARD 20)
endif()