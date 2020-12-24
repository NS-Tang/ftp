cmake_minimum_required (VERSION 2.8)
project (mft)
# 设置为debug模式
SET(CMAKE_BUILD_TYPE "Debug")
# 设置gcc在项目为debug模式下的选项，如需release也可单独设置，这个实验就不提供了。
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -Og -Wall -g3 -ggdb3")

# 生成客户端ft的目标文件，其中lib目录下是给的代码，第一个参数是目标，下同。
# 库文件中使用errono的地方定义为外部变量的都改为#include <errno.h>，前一种方式会产生难以解决的编译错误且多线程下只有一个errno
add_executable(ft ft_main.c lib/connectTCP.c lib/connectsock.c lib/errexit.c)
# 生成多进程服务端的目标文件
add_executable(mpftd ftd_main.c mpftd.c ftd.c lib/passiveTCP.c lib/passivesock.c lib/errexit.c)
# 寻找线程库，这里在Linux下预期找到的是pthread，注意直接在gcc的选项里设置是不行的，必须通过这里加入库。
find_package(Threads)
# 生成多线程服务端
add_executable(mtftd ftd_main.c mtftd.c ftd.c lib/passiveTCP.c lib/passivesock.c lib/errexit.c)
target_link_libraries(mtftd ${CMAKE_THREAD_LIBS_INIT})
