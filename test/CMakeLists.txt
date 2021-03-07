include(CTest)

add_executable(
    uhttp_test_arraylist "../src/list.c" "./test_common.c" "./list.c"
)
target_include_directories(uhttp_test_arraylist PRIVATE "." "../src")
target_compile_definitions(uhttp_test_arraylist PRIVATE "_UHTTP_TEST_STANDALONE_")
add_test(NAME "Array List Test" COMMAND uhttp_test_arraylist)
