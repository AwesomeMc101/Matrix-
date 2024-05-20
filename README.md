# MatrixPlusPlus
* Generate any sort of matrix you want using vectors.
* Legit just two files, a header and object file.

# Basics
* To define a matrix, simply type ```Matrix m```.
* You can construct a 2x2 matrix by calling something like this:
```Matrix m({ {1,2}, {3,4} }) ```
* The constructor does not have a default check to ensure each row/column is numerically
aligned. You can build that in or add checks.

# Operators
* Matrices have the +-* operators all built in. These operators support matrix
upon matrix arithmetic ```(m_1 * m_2)``` and matrix on integer arithmetic ```(m_1 * 3)```
* The * operator performs DOT multiplication, the true matrix multiplication.

# Transposition
* Following the lead of Numpy, you can transpose the matrix by calling ```.T()```.
* Transposition inverts the rows and columns.

# Determinant
* Calling ```.determinant()``` will return the determinant of the matrix.
* As per linear algebra rules, this function will only work on a matrix with a square shape.
* If you want to add a check in your code to prevent determinant failure, you can do something
like the following:
```if(m.is_square())``` which is an already built-in call.

