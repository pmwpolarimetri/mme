add_library(lumenera_sdk SHARED IMPORTED GLOBAL)
set_target_properties(lumenera_sdk PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files (x86)/Lumenera Corporation/LuCam Capture Software/SDK/Include")
set_target_properties(lumenera_sdk PROPERTIES IMPORTED_IMPLIB "C:/Program Files (x86)/Lumenera Corporation/LuCam Capture Software/SDK/lib/lib64/lucamapi.lib")
set_target_properties(lumenera_sdk PROPERTIES IMPORTED_LOCATION "C:/Program Files (x86)/Lumenera Corporation/LuCam Capture Software/SDK/lib/lib64/lucamapi.dll")
