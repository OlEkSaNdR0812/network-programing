#include "main.h"

void usage(const char* exe_name)
{
    printf("Usage:\n");
    printf("\t%s -h <host> -p <port>\n", exe_name);
}

int start(int argc, char* argv[])
{
    char host[2048] = "";
    int port;

    if (argc >= 3)
    {
        char arg_line[4 * 1024] = "";
        combine_arg_line(arg_line, argv, 1, argc);
        int ret = sscanf(arg_line, "-h %s -p %d", host, &port);

        if (ret < 2) {
            usage(argv[0]);
            return -1;
        }
    }
    else {
        printf("Enter server address (-h <host> -p <port>): ");
        int ret = scanf("-h %s -p %d", host, &port);

        if (ret < 2) {
            usage(argv[0]);
            return -2;
        }
    }

    return init_client(host, port);
}

int init_client(const char* host, short port)
{
    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_socket <= 0)
    {
        printf("Cannot create client socket\n");
        return -1;
    }

    printf("Socket created\n");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    char target_host[2048] = "";
    resolve_address(host, target_host);

    server_address.sin_addr.s_addr = inet_addr(target_host);

    printf("Prepared to send requests to server: %s:%d\n", target_host, port);

    return process_connection(client_socket, server_address);
}

int process_connection(SOCKET client_socket, struct sockaddr_in server_address)
{
    char buffer[4 * 1024] = "";
    struct PolynomialRequest request;
    struct PolynomialResponse response;

    printf("Choose request type:\n");
    printf("1. Evaluate polynomial at x\n");
    printf("2. Get polynomial order\n");
    printf("3. Get polynomial coefficients\n");
    printf("Enter choice: ");
    int choice;
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
        request.type = EVALUATE_POLYNOMIAL;
        printf("Enter value of x: ");
        scanf("%lf", &request.x);
        break;
    case 2:
        request.type = GET_ORDER;
        break;
    case 3:
        request.type = GET_COEFFICIENTS;
        break;
    default:
        printf("Invalid choice\n");
        return -1;
    }

    int len = sizeof(server_address);
    int ret = sendto(client_socket, (char*)&request, sizeof(request), 0, (struct sockaddr*)&server_address, len);
    if (ret <= 0)
    {
        printf("Error sending request\n");
        return -1;
    }

    printf("====> Sent [%d bytes]\n", ret);

    memset(&response, 0, sizeof(response));
    ret = recvfrom(client_socket, (char*)&response, sizeof(response), 0, (struct sockaddr*)&server_address, &len);
    if (ret <= 0)
    {
        printf("Error receiving response\n");
        return -1;
    }

    printf("<==== Received [%d bytes]\n", ret);
    print_polynomial_response(&response);

    return closesocket(client_socket);
}

void print_polynomial_response(struct PolynomialResponse* response)
{
    if (response->result != 0) {
        printf("Polynomial evaluated result: %lf\n", response->result);
    }
    if (response->order != 0) {
        printf("Polynomial order: %d\n", response->order);
    }
    if (response->coefficients[0] != 0) {
        printf("Polynomial coefficients: ");
        for (int i = 0; i <= response->order; ++i) {
            printf("%lf ", response->coefficients[i]);
        }
        printf("\n");
    }
}
