ע������
����clientʱ��Ҫ����PC��ִ��iperf -s -i 1 -u -U����ִ��AT+NCSTART������򣬿��ܵ���PC���ղ������ݡ�
����serverʱ������ִ��AT+NSSTART�������PC��ִ��iperf -c ������򣬿��ܵ��½��ն����ݶ�ʧ��

����������裺
1.��¼bin
2.���ӵ�·��
    AT+WSCONN="a","qqqqqqqq"     ����wifi������Ϊ�û��������룬�Ǽ���ʱ���Բ���������
3. udp client��������
    AT+NCSTART=192.168.1.102,10  �����ֱ�Ϊserver address��PC�˵�IP��ַ��,����ʱ�䣨��λΪ�룩
    ���Խ�������+ok��
    Server����ʹ������Ĳ������
    iperf -s -i 1 -u -U
4. udp server��������
    AT+NSSTART
    ���Խ�������+ok��
5. multcast client��������
    AT+MCSTART=225.0.0.1,10     �����ֱ�Ϊmultcast address��225.0.0.1��,����ʱ�䣨��λΪ�룩
    server�˶�Ӧ�Ĳ����������£�
    iperf -s -u -B 225.0.0.1 -i 1
6. multcast server��������
    AT+MSSTART=225.0.0.1        ����Ϊmultcast address��225.0.0.1��
    ���Խ�������+ok��
    client�˶�Ӧ�Ĳ����������£�
    iperf -c 225.0.0.1 -u -b 80k -t 10 -i 1
7. ���Ҫ��������client��server,ÿ�β��������Ӧ�Ĳ�������ɡ�