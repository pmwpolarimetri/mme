
set(imaging_header_dir "${CMAKE_CURRENT_SOURCE_DIR}/include/mme/imaging")

add_library(imaging INTERFACE "${imaging_header_dir}/image.h"  )
add_library(mme::imaging ALIAS imaging)
target_include_directories(imaging INTERFACE include)
target_compile_features(imaging INTERFACE cxx_std_20)

#if (MSVC)
	# There is a bug in visual studio that prevents intellisense from realizing
	# /std:c++latest is on the command line if you only use target_compile_features(cxx_std_20)
	#target_compile_options(imaging INTERFACE "/std:c++latest")
#endif()
