add_library(nidaqmx SHARED IMPORTED GLOBAL)
set_target_properties(nidaqmx PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/include")
set_target_properties(nidaqmx PROPERTIES IMPORTED_IMPLIB "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib64/msvc/NIDAQmx.lib")
