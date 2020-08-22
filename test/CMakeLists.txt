include_directories (${TEST_SOURCE_DIR}/src)
set(CMAKE_C_STANDARD 99)

add_executable (searchTest test.c ../src/buffer.c ../src/recycler.c ../src/bufferarray.c ../src/log.c ../src/hashtable.c)
add_test (NAME searchTest COMMAND searchTest)