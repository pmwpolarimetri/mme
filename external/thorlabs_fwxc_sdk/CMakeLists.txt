add_library(thorlabs_fwxc_sdk SHARED IMPORTED GLOBAL)
set_target_properties(thorlabs_fwxc_sdk PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files (x86)/Thorlabs/FWxC/Sample/Thorlabs_FWxC_C++SDK")
set_target_properties(thorlabs_fwxc_sdk PROPERTIES IMPORTED_IMPLIB "C:/Program Files (x86)/Thorlabs/FWxC/Sample/Thorlabs_FWxC_C++SDK/FilterWheel102_win64.lib")
set_target_properties(thorlabs_fwxc_sdk PROPERTIES IMPORTED_LOCATION "C:/Program Files (x86)/Thorlabs/FWxC/Sample/Thorlabs_FWxC_C++SDK/FilterWheel102_win64.dll")
