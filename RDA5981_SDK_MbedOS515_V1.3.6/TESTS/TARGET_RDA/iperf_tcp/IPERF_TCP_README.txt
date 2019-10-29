注意事项
测试client时，要先在PC端执行iperf -s -i 1，再执行AT+NCSTART命令。否则，可能导致PC端收不到数据。

tcp iperf具体操作步骤：
1.烧录bin文件。
2.连接到路由
    AT+WSCONN="a","qqqqqqqq"     连接wifi，参数为用户名，密码，非加密时可以不输入密码
3. tcp client测试命令
    AT+NCSTART=192.168.1.102,10  参数分别为server address（PC端的IP地址）,测试时间（单位为秒）
    测试结束返回+ok。
    如果不输入时间，默认时间为无穷大。
4. tcp server测试命令
    AT+NSSTART
    测试结束返回+ok。
5. 如果要轮流测试client和server,每次测试输入对应的测试命令即可。
