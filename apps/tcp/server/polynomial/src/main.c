#include "main.h"

SOCKET server_socket = -1;

void free_socket()
{
	if (server_socket > 0)
	{
		closesocket(server_socket);
	}
}
void usage(const char* exe_name)
{
	printf("Usage:\n");
	printf("\t%s -p <port> -q <que_size>", exe_name);
}

int start(int argc, char* argv[])
{
	int port = DEFAULT_PORT;

	int queue_size = DEFAULT_QUEUE;

	if (argc >= 3)
	{
		char arg_line[128];
		
		memset(arg_line, 0, sizeof(arg_line));

		combine_arg_line(arg_line, argv, 1, argc);

		int ret = sscanf(arg_line, "-p %d -q %d", &port, &queue_size);

		if (ret < 1) {
			usage(argv[0]);
			return -1;
		}
	}

	return init_client(port, queue_size);
}

int init_client(short port, int queue_size)
{
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (server_socket <= 0)
	{
		printf("Cannot create socket\n");
		return -1;
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr*)&address, sizeof(address))) {
		printf("Cannot bind socket to port %d\n", port);
		return -2;
	}
	
	if (listen(server_socket, queue_size))
	{
		printf("Cannot listen socket on port %d\n", port);
		return -3;
	}

	printf("Sever run on port %d\n", port);

	return process_connection();
}

int process_connection()
{
	SOCKET client_socket = -1;

	while (1)
	{
		struct sockaddr_in client_addr;
		int len = sizeof(client_addr);

		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &len);
		if (client_socket <= 0)
		{
			printf("Error incoming connection\n");
			continue;
		}

		printf("Connection established from: %s\n", inet_ntoa(client_addr.sin_addr));

		struct PolynomialRequest request;
		int ret = recv(client_socket, (char*)&request, sizeof(request), 0);
		if (ret <= 0)
		{
			printf("Receiving data error\n");
			continue;
		}

		printf("<==== Received: [%d bytes]\n", ret);

		struct PolynomialResponse response;
		process_request(&request, &response);

		ret = send(client_socket, (char*)&response, sizeof(response), 0);
		if (ret <= 0)
		{
			printf("Sending data error\n");
			continue;
		}

		printf("====> Sent: [%d bytes]\n", ret);
	}

	if (client_socket > 0)
	{
		return closesocket(client_socket);
	}

	return 0;
}
	
double coefficients[] = { 1, -2, 3 };  
int order = sizeof(coefficients) / sizeof(coefficients[0]) - 1;

int process_request(struct PolynomialRequest* request, struct PolynomialResponse* response)
{
	switch (request->type) {
	case EVALUATE_POLYNOMIAL:
		response->result = evaluatePolynomialImpl(request->x);
		printf("Processed polynomial evaluation. Result: %lf\n", response->result);
		break;

	case GET_ORDER:
		response->order = order;
		printf("Processed polynomial order request. Order: %d\n", response->order);
		break;

	case GET_COEFFICIENTS:
		memcpy(response->coefficients, coefficients, sizeof(coefficients));
		printf("Processed polynomial coefficients request.\n");
		break;

	default:
		printf("Unknown request type.\n");
		break;
	}

	return 0;
}

double evaluatePolynomialImpl(double x){
	double result = 0;
	for (int i = 0; i <= order; i++) {
		result = result * x + coefficients[i];
	}
	return result;
}