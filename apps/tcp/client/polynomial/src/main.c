#include "main.h"
#include "winsock2.h"

#define MAX_CLIENTS 100

typedef struct {
    char host[2048];
    int port;
    double x_value;
} ClientData;

void usage(const char* exe_name)
{
    printf("Usage:\n");
    printf("\t%s -h <host> -p <port> -n <number_of_clients> -f <input_file>\n", exe_name);
}

int process_connection(ClientData* client_data);

int start(int argc, char* argv[])
{
    char host[2048] = "";
    int port = 0;
    int num_clients = 1; 
    char file_name[2048] = "";

    if (argc >= 7)
    {
        char arg_line[4 * 1024] = "";
        combine_arg_line(arg_line, argv, 1, argc);
        int ret = sscanf(arg_line, "-h %s -p %d -n %d -f %s", host, &port, &num_clients, file_name);

        if (ret < 4) {
            usage(argv[0]);
            return -1;
        }
    }
    else {
        usage(argv[0]);
        return -1;
    }

    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        printf("Cannot open input file %s\n", file_name);
        return -1;
    }

    thrd_t threads[MAX_CLIENTS];
    ClientData client_data[MAX_CLIENTS];

    for (int i = 0; i < num_clients; i++) {
        fscanf(file, "%lf", &client_data[i].x_value);
        strcpy(client_data[i].host, host);
        client_data[i].port = port;
        thrd_create(&threads[i], (thrd_start_t)process_connection, &client_data[i]);
    }

    fclose(file);

    for (int i = 0; i < num_clients; i++) {
        thrd_join(threads[i], NULL);
    }

    return 0;
}

int process_connection(ClientData* client_data)
{
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket <= 0)
    {
        printf("Cannot create client socket. Error: %d\n", WSAGetLastError());
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(client_data->port);
    server_address.sin_addr.s_addr = inet_addr(client_data->host);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        printf("Cannot connect to server %s:%d. Error: %d\n", client_data->host, client_data->port, WSAGetLastError());
        return -1;
    }

    struct PolynomialRequest request;
    struct PolynomialResponse response;

    memset(&request, 0, sizeof(request));
    request.x = client_data->x_value;
    request.type = EVALUATE_POLYNOMIAL;

    if (send_polynomial_request(client_socket, &request, &response) == 0) {
        printf("Polynomial evaluated result: %lf\n", response.result);
    }
    else {
        printf("Error processing polynomial evaluation request\n");
    }

    request.type = GET_ORDER;

    if (send_polynomial_request(client_socket, &request, &response) == 0) {
        printf("Polynomial order: %d\n", response.order);
    }
    else {
        printf("Error processing polynomial order request\n");
    }

    request.type = GET_COEFFICIENTS;

    if (send_polynomial_request(client_socket, &request, &response) == 0) {
        printf("Polynomial coefficients: ");
        for (int i = 0; i <= response.order; ++i) {
            printf("%lf ", response.coefficients[i]);
        }
        printf("\n");
    }
    else {
        printf("Error processing polynomial coefficients request\n");
    }

    closesocket(client_socket);
    return 0;
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
