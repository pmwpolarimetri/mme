add_library(ihr "ihr.cpp" "include/mme/ihr/ihr.h"  )
add_library(mme::ihr ALIAS ihr)
target_include_directories(ihr PUBLIC include)
target_compile_features(ihr PUBLIC cxx_std_20)