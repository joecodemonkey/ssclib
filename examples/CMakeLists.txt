include_directories (../src)
set(CMAKE_C_STANDARD 99)

#add_executable (searchFile searchFile/main.c ../src/buffer.c ../src/recycler.c ../src/bufferarray.c ../src/log.c ../src/hashtable.c)

add_executable(searchFile searchFile/main.c)

target_link_libraries(searchFile ${CMAKE_SOURCE_DIR}/src/libssc.a)
