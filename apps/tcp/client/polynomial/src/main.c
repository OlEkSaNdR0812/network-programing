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
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

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

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address))) {
        printf("Cannot connect to server %s:%d\n", target_host, port);
        return -2;
    }

    printf("Success connection to server: %s:%d\n", target_host, port);

    return process_connection(client_socket);
}

int process_connection(SOCKET client_socket)
{
    struct PolynomialRequest request;
    struct PolynomialResponse response;

    int choice;
    printf("Choose request type:\n");
    printf("1. Evaluate polynomial at x\n");
    printf("2. Get polynomial order\n");
    printf("3. Get polynomial coefficients\n");
    printf("Enter choice: ");
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

    if (send_polynomial_request(client_socket, &request, &response) == 0)
    {
        print_polynomial_response(&response);
    }

    return closesocket(client_socket);
}

int send_polynomial_request(SOCKET socket, struct PolynomialRequest* request, struct PolynomialResponse* response)
{
    int ret = send(socket, (char*)request, sizeof(*request), 0);
    if (ret <= 0)
    {
        printf("Error sending request\n");
        return -1;
    }

    ret = recv(socket, (char*)response, sizeof(*response), 0);
    if (ret <= 0)
    {
        printf("Error receiving response\n");
        return -1;
    }

    return 0;
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