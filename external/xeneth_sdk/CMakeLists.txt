add_library(xeneth_sdk STATIC IMPORTED GLOBAL)
set_target_properties(xeneth_sdk PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/Xeneth/Sdk/Include")
#set_target_properties(xeneth_sdk PROPERTIES IMPORTED_IMPLIB "C:/Program Files/Xeneth/Sdk/Lib/xeneth64.lib")
set_target_properties(xeneth_sdk PROPERTIES IMPORTED_LOCATION "C:/Program Files/Xeneth/Sdk/Lib/xeneth64.lib")
