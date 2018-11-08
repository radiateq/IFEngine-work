project(test NONE)
    cmake_minimum_required(VERSION 2.8.0)
    set (CMAKE_Fortran_FLAGS "")
    set (CMAKE_EXE_LINKER_FLAGS "-Wl,--fix-cortex-a8  -fPIE -pie -Wl,--gc-sections -Wl,-z,nocopyreloc")
    enable_language(Fortran)
  