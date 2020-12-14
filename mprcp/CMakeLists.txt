cmake_minimum_required (VERSION 2.8)
project (mpftp)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Og -Wall -g3 -ggdb3")

add_executable(mpftpd mpftpd.c ../lib/passiveTCP.c ../lib/passivesock.c ../lib/errexit.c)
add_executable(ftp ftp.c ../lib/connectTCP.c ../lib/connectsock.c ../lib/errexit.c)
