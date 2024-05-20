#include <vector>
#include <math.h>
#include <algorithm>
#include <numeric>

#define _PP_USE_THREADS

#ifdef _PP_USE_THREADS
#include <Windows.h>
#endif

#define _DEBUG
#ifdef _DEBUG
#include <iostream>
#endif

typedef long double NINT; //matrix double

typedef struct Shape {
	unsigned int rows, cols;
};

typedef class Matrix {
private:
	std::vector<std::vector<NINT>> rows{};
	Shape shape;
public:
	/* possible constructors */

	/* 3d/full matrix */
	Matrix(std::vector<std::vector<NINT>> fullmatrix) { 
		rows = fullmatrix;
		sh();
	}//3d

	void sh(); //set the shape
	void T(); //transpose

	std::vector<std::vector<NINT>> grab_matrix() { return rows; } //debug
	 
	Matrix operator+(const Matrix); //add
	Matrix operator-(const Matrix); //sub
	Matrix operator*(const Matrix); //multiply
};

