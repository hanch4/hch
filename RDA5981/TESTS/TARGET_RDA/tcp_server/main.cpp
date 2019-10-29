#include "mbed.h"
#include "rtos.h"
#include "WiFiStackInterface.h"
#include <stdio.h>
#include <string.h>

#define TCP_SERVER_PORT       8000
#define BUF_LEN               1024

WiFiStackInterface wifi;
TCPServer server;
TCPSocket client;
SocketAddress client_addr;

char buffer[BUF_LEN+1];
unsigned char send_content[] = "Hello World!\n";

int main (void)
{
    printf("Start test case TCPServer...\n");

    wifi.start_ap("RDA_TCP_SERVER", NULL, 9);
    Thread::wait(5000);
    printf("IP Address is %s\n", wifi.get_ip_address_ap());
    server.open(&wifi);
    server.bind(wifi.get_ip_address(), TCP_SERVER_PORT);

    server.listen(5);
    printf("Server Listening\n");

    while (true) {
        printf("\nWait for new connection...\r\n");
        server.accept(&client, &client_addr);

        printf("Connection from: %s:%d\r\n", client_addr.get_ip_address(), client_addr.get_port());
        while(true) {
            int n = client.recv(buffer, sizeof(buffer));
            if(n <= 0)
                break;
            printf("Recieved Data: %d\r\n%.*s\r\n", n, n, buffer);
            client.send(send_content, sizeof(send_content) - 1);
        }
        client.close();
    }
}


