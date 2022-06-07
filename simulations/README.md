# 在你本机运行所需的简单配置
* 在 omnetpp.ini 文件中，修改第二行 ned-path 的值

    ned-path 指定仿真所依赖的 ned 文件搜索目录。这里给了三个目录，前两个是相对路径，不用修改。你只需要修改第三个目录，将其替换为你本地 Inet 库的 src 目录路径。

* 编译仿真程序
    1. 生成 Makefile 文件
    opp_makemake -f -o (这里指定仿真程序名) -I(这里写你本地 Inet库 src/inet 目录) -L(这里写你本地 Inet库 src 目录) -lINET。
    2. make
