add_library(ztour "ztour.cpp" "memory.cpp" "decoder.h" "decoder.cpp" "encoder.h" "encoder.cpp" "mutex_holder.h" "hook_inst_inner.h" "hook_inst_inner.cpp" "mem_page.h" "mem_page.cpp" "base.cpp" "hook_inst.cpp"
        atomic_call_gate.h
        relocation.cpp
        relocation.h
)

# TODO: Make sure this is the best way to do this (?)
target_include_directories(ztour
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(ztour PRIVATE cxx_std_17)

add_subdirectory(zydis)
target_link_libraries(ztour PRIVATE zydis)