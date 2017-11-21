set (CMAKE_C_FLAGS -nostdlib)
include_directories(../usermode_library)
add_executable(minesweeper minesweeper/minesweeper.c)