#pragma once

struct QuadraticEquation
{
	double a;
	double b;
	double c;
};

enum SquareRootResult {NO_ROOT, ONE_ROOT, TWO_ROOT};

struct SquareRootData
{
	enum SquareRootResult result;

	double x1;
	
	double x2;
};

enum NumberOperationType { AVG, MAX, MIN };

struct NumberOperationData
{
	enum NumberOperationType type;

	double data [1024];
};

struct NumberOperationResult
{
	enum NumberOperationType type;

	double result;
};

enum RequestType { EVALUATE_POLYNOMIAL, GET_ORDER, GET_COEFFICIENTS };

struct PolynomialRequest {
	enum RequestType type;
	double x;  
};


struct PolynomialResponse {
	double result;  
	double coefficients[10];  
	int order;  
};