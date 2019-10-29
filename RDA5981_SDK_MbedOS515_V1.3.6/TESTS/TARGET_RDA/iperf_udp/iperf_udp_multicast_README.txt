注意事项
测试client时，要先在PC端执行iperf -s -i 1 -u -U，再执行AT+NCSTART命令。否则，可能导致PC端收不到数据。
测试server时，请先执行AT+NSSTART命令，再在PC端执行iperf -c 命令。否则，可能导致接收端数据丢失。

具体操作步骤：
1.烧录bin
2.连接到路由
    AT+WSCONN="a","qqqqqqqq"     连接wifi，参数为用户名，密码，非加密时可以不输入密码
3. udp client测试命令
    AT+NCSTART=192.168.1.102,10  参数分别为server address（PC端的IP地址）,测试时间（单位为秒）
    测试结束返回+ok。
    Server端请使用下面的测试命令：
    iperf -s -i 1 -u -U
4. udp server测试命令
    AT+NSSTART
    测试结束返回+ok。
5. multcast client测试命令
    AT+MCSTART=225.0.0.1,10     参数分别为multcast address（225.0.0.1）,测试时间（单位为秒）
    server端对应的测试命令如下：
    iperf -s -u -B 225.0.0.1 -i 1
6. multcast server测试命令
    AT+MSSTART=225.0.0.1        参数为multcast address（225.0.0.1）
    测试结束返回+ok。
    client端对应的测试命令如下：
    iperf -c 225.0.0.1 -u -b 80k -t 10 -i 1
7. 如果要轮流测试client和server,每次测试输入对应的测试命令即可。