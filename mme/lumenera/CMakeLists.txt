add_library(lumenera "lumeneracamera.cpp" "include/mme/lumenera/lumeneracamera.h")
add_library(mme::lumenera ALIAS lumenera)
target_link_libraries(lumenera PUBLIC mme::imaging PRIVATE lumenera_sdk)
target_include_directories(lumenera PUBLIC include)
target_compile_features(motion PUBLIC cxx_std_20)

