add_executable(open_jtalk open_jtalk.c)
set_target_properties(open_jtalk PROPERTIES LINKER_LANGUAGE CXX)
install(TARGETS open_jtalk DESTINATION bin)
