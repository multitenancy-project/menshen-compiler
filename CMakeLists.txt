# Makefile for the p416-merge

# sources for p416-merge
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

set (P4C_FPGA_SRCS
	options.h
    main.cpp
    )
add_cpplint_files (${CMAKE_CURRENT_SOURCE_DIR} "${P4C_FPGA_SRCS}")

add_executable(p4c-fpga ${P4C_FPGA_SRCS})
# message (STATUS "libs ${P4C_LIBRARIES}")
# message (STATUS "deps ${P4C_LIB_DEPS}")
target_link_libraries (p4c-fpga ${P4C_LIBRARIES} ${P4C_LIB_DEPS})
add_dependencies(p4c-fpga genIR frontend)

install(TARGETS p4c-fpga RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})

add_custom_target(linkp4cfpga
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/p4c-fpga ${P4C_BINARY_DIR}/p4c-fpga
  # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/p4include ${P4C_BINARY_DIR}/p4include
  # COMMAND ${CMAKE_COMMAND} -E create_symlink ${P4C_BINARY_DIR}/p4include ${CMAKE_CURRENT_BINARY_DIR}/p4include
  # COMMAND ${CMAKE_COMMAND} -E create_symlink ${P4C_BINARY_DIR}/p4_14include ${CMAKE_CURRENT_BINARY_DIR}/p4_14include
  )
add_dependencies(p4c_driver linkp4cfpga)

