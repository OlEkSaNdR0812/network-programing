#pragma once

#include "stdio.h"
#include "string.h"
#include "net-utils.h"
#include "shared-data.h"

int start(int argc, char* argv[]);

void usage(const char* exe_name);

int init_client(const char* host, short port);

int process_connection(SOCKET client_socket, struct sockaddr_in server_address);

//int process_request(struct QuadraticEquation* request, struct SquareRootData* response);
int process_request(struct PolynomialRequest* request, struct PolynomialResponse* response);

int send_polynomial_request(SOCKET socket, struct PolynomialRequest* request, struct PolynomialResponse* response);

void print_polynomial_response(struct PolynomialResponse* response);

