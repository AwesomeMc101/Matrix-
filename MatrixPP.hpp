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

	bool is_square() { return (shape.rows == shape.cols); }

	unsigned int get_rows() { return shape.rows; }
	unsigned int get_cols() { return shape.cols; }

	NINT determinant();
	NINT tr();

	std::vector<std::vector<NINT>> grab_matrix() { return rows; } //debug
	 
	Matrix operator+(const Matrix); //add 2 matrices
	Matrix operator-(const Matrix); //sub 2 matrices
	Matrix operator*(const Matrix); //multiply 2 matrices

	Matrix operator+(const NINT); //add 2 matrices
	Matrix operator-(const NINT); //sub 2 matrices
	Matrix operator*(const NINT); //multiply 2 matrices
};

NINT trace(Matrix);
