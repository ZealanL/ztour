add_library(zydis STATIC "Zydis.c")
target_include_directories(zydis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(zydis PUBLIC ZYDIS_STATIC_BUILD)