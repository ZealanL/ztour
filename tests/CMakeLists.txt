add_executable(ztour_test testmain.cpp)
target_link_libraries(ztour_test PRIVATE ztour)
add_test(NAME ztour_main_test COMMAND ztour_test)