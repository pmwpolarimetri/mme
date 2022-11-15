add_library(nidaq "adcnidaq.cpp" "nidaqerrors.cpp" "include/mme/nidaq/adcnidaq.h" "include/mme/nidaq/nidaqerrors.h" )
add_library(mme::nidaq ALIAS nidaq)
target_link_libraries(nidaq PRIVATE nidaqmx)
target_include_directories(nidaq PUBLIC include)
target_compile_features(nidaq PUBLIC cxx_std_20)

