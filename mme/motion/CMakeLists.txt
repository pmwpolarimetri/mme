add_library(motion "espdriver.cpp" "include/mme/motion/espdriver.h")
add_library(mme::motion ALIAS motion)
target_link_libraries(motion PUBLIC asio)
target_include_directories(motion PUBLIC include)
target_compile_features(motion PUBLIC cxx_std_20)


