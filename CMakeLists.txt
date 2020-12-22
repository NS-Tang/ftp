cmake_minimum_required (VERSION 2.8)
project (mft)
SET(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Og -Wall -g3 -ggdb3")


add_executable(ft ft_main.c lib/connectTCP.c lib/connectsock.c lib/errexit.c)
add_executable(mpftd ftd_main.c mpftd.c ftd.c lib/passiveTCP.c lib/passivesock.c lib/errexit.c)
find_package(Threads)
add_executable(mtftd ftd_main.c mtftd.c ftd.c lib/passiveTCP.c lib/passivesock.c lib/errexit.c)
target_link_libraries(mtftd ${CMAKE_THREAD_LIBS_INIT})
