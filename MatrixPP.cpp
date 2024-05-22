#include "MatrixPP.hpp"

#ifdef _DEBUG
template<typename T>
void printfvector(std::vector<T> vec)
{
	printf("\n{ ");
	for (int i = 0; i < vec.size(); i++) {
		std::cout << vec[i] << " ";
	}
	printf("}");
}
#endif


template<typename T>
static Shape set_shape(std::vector<std::vector<T>> provided_vector) {

	Shape ns;
	ns.rows = provided_vector.size();
	if (ns.rows) {
		ns.cols = provided_vector[0].size();
	}
	return ns;
}

void Matrix::sh() {
	this->shape = set_shape(this->rows);
}

#ifdef _PP_USE_THREADS
namespace Tranpose_Thread {

	typedef struct tt_pass_data {
		std::vector<std::vector<NINT>> old_matrix;
		int col;
		std::vector<NINT>* to_write;
		int* threads;
	};
	void tranpose_thread_set_new_row(LPVOID pd) {
		
		tt_pass_data ttpd = *(static_cast<tt_pass_data*>(pd));
		//ttpd.threads += 1;
		//trust that the col is real
		for (std::vector<NINT> row : ttpd.old_matrix) {
			ttpd.to_write->push_back(row[ttpd.col]);
		}
		*ttpd.threads -= 1;
	}
}

using Tranpose_Thread::tt_pass_data;
#endif

void Matrix::T() {
#ifdef _PP_USE_THREADS
	std::vector<std::vector<NINT>> new_m;
	new_m.resize(this->shape.cols);

	int threads = 0;
	for (int i = 0; i < shape.cols; i++) {
		tt_pass_data* ttpd = new tt_pass_data; //(tt_pass_data*)malloc(sizeof(tt_pass_data));
		ttpd->col = i;
		ttpd->old_matrix = this->rows;
		ttpd->to_write = &new_m[i];
		ttpd->threads = &threads;
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)Tranpose_Thread::tranpose_thread_set_new_row, (LPVOID) & *ttpd, 0, 0);
	}
	while (threads != 0) {} //await completion 
	this->rows = new_m;
	this->sh();
	return;
#else 
	/* regular

	-> this could honestly be faster but i don't feel like doing a speedtest!

	*/

	std::vector<std::vector<NINT>> new_m{};
	new_m.resize(shape.cols);
	for (std::vector<NINT> row : rows) {
		for (int i = 0; i < shape.cols; i++) {
			new_m[i].push_back(row[i]);
		}
	}
	this->rows = new_m;
	this->sh();
	return;
#endif
}

static std::vector<NINT> multiply_vectors(std::vector<NINT> a, std::vector<NINT> b) {
	if (a.size() != b.size()) { printf("mverrr"); return a; }
	std::vector<NINT> c{};
	for (int i = 0; i < a.size(); i++) {
		c.emplace_back((a[i] * b[i]));
	}
	return c;
}
static NINT sum_vector(std::vector<NINT> flt) {
	return (std::accumulate(flt.begin(), flt.end(), 0.0f));
}
static std::vector<NINT> add_vectors(std::vector<NINT> a, std::vector<NINT> b) {
	if (a.size() != b.size()) { printf("averrr"); return a; }
	std::vector<NINT> c{};
	for (int i = 0; i < a.size(); i++) {
		c.emplace_back((a[i] + b[i]));
	}
	return c;
}
static std::vector<NINT> sub_vectors(std::vector<NINT> a, std::vector<NINT> b) {
	if (a.size() != b.size()) { printf("sverrr"); return a; }
	std::vector<NINT> c{};
	for (int i = 0; i < a.size(); i++) {
		c.emplace_back((a[i] - b[i]));
	}
	return c;
}


#ifdef _PP_USE_THREADS
namespace Threaded_Matrix_Multiplication {
	typedef struct mt_passdata {
		std::vector<NINT> row;
		std::vector<std::vector<NINT>> multiplier;

		std::vector<NINT>* write_ptr;
		int* threads;
	};

	void thread_multiply_row(LPVOID pd) {
		mt_passdata mtpd = *(static_cast<mt_passdata*>(pd));

		for (std::vector<NINT> col : mtpd.multiplier) {
			mtpd.write_ptr->push_back(sum_vector(multiply_vectors(mtpd.row, col)));
		}
		*mtpd.threads -= 1;
	}
}
#endif

Matrix Matrix::operator*(const Matrix nm) {
	/* multiplication uses linear alegbra's TRANPOSITION (.T()) to speed up the calculations*/

	if (this->shape.cols != nm.shape.rows) {
		return *this;
	}
#ifdef _PP_USE_THREADS
	
	std::vector<std::vector<NINT>> new_matrix_vector{};
	new_matrix_vector.resize(rows.size());

	Matrix z = nm;
	z.T(); //switch rows and columns
	std::vector<std::vector<NINT>> multiplier = z.grab_matrix();

	int threads = 0;
	for(int i = 0; i < rows.size(); i++){

		std::vector<NINT> row = rows[i];

		Threaded_Matrix_Multiplication::mt_passdata* mtpd = new Threaded_Matrix_Multiplication::mt_passdata;
		mtpd->multiplier = multiplier;
		mtpd->threads = &threads;
		mtpd->write_ptr = &new_matrix_vector[i];
		mtpd->row = row;
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)Threaded_Matrix_Multiplication::thread_multiply_row, (LPVOID)(&*mtpd), 0, 0);
	}

	while(threads != 0){}
	Matrix f(new_matrix_vector);
	return f;
#else
	std::vector<std::vector<NINT>> new_matrix_vector{};
	new_matrix_vector.resize(rows.size());

	Matrix z = nm;
	z.T(); //switch rows and columns
	std::vector<std::vector<NINT>> multiplier = z.grab_matrix();
	for (int i = 0; i < rows.size(); i++) {
		std::vector<NINT> row = rows[i];

		for (std::vector<NINT> col : multiplier) {
			new_matrix_vector[i].push_back(sum_vector(multiply_vectors(row, col)));
		}
	}
	Matrix f(new_matrix_vector);
	return f;
#endif

	return nm; //even possible? probably not, but computers are random

}

#ifdef _PP_USE_THREADS
namespace Threaded_ArithOp {
	typedef struct ta_passdata {
		std::vector<NINT> a, b;
		std::vector<NINT>* write_ptr;
		int* threads;

		bool op; //1=add,0=sub
	};

	void threaded_runop(LPVOID pd) {
		ta_passdata tapd = *(static_cast<ta_passdata*>(pd));

		if (tapd.op == 1) {
			for(int i = 0; i < tapd.a.size(); i++){
				tapd.write_ptr->emplace_back((tapd.a[i] + tapd.b[i]));
			}
		}
		else {
			for (int i = 0; i < tapd.a.size(); i++) {
				tapd.write_ptr->emplace_back((tapd.a[i] - tapd.b[i]));
			}
		}

		*tapd.threads -= 1;
		return;
	}
}

#endif

Matrix Matrix::operator+(const Matrix nm) {
	if (shape.rows != nm.shape.rows ||
		shape.cols != nm.shape.cols) {
		return *this;
	}

#ifdef _PP_USE_THREADS

	std::vector<std::vector<NINT>> new_m{};
	new_m.resize(rows.size());

	int threads = 0;
	for (int i = 0; i < nm.shape.rows; i++) {
		Threaded_ArithOp::ta_passdata* tapd = new Threaded_ArithOp::ta_passdata;
		tapd->a = rows[i];
		tapd->b = nm.rows[i];
		tapd->write_ptr = &new_m[i];
		tapd->threads = &threads;
		tapd->op = 1; //ADD
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)(Threaded_ArithOp::threaded_runop), (LPVOID)(&*tapd), 0, 0);
	}
	while (threads) {}

	Matrix r(new_m);
	return r;
#else
	std::vector<std::vector<NINT>> new_m{};
	new_m.resize(rows.size());

	for (int i = 0; i < nm.shape.rows; i++) {
		std::vector<NINT> a = rows[i];
		std::vector<NINT> b = nm.rows[i];

		for (int x = 0; x < a.size(); x++) {
			new_m[i].emplace_back((a[x] + b[x]));
		}
	}

	Matrix r(new_m);
	return r;
#endif

}
Matrix Matrix::operator-(const Matrix nm) {
	if (shape.rows != nm.shape.rows ||
		shape.cols != nm.shape.cols) {
		return *this;
	}

#ifdef _PP_USE_THREADS

	std::vector<std::vector<NINT>> new_m{};
	new_m.resize(rows.size());

	int threads = 0;
	for (int i = 0; i < nm.shape.rows; i++) {
		Threaded_ArithOp::ta_passdata* tapd = new Threaded_ArithOp::ta_passdata;
		tapd->a = rows[i];
		tapd->b = nm.rows[i];
		tapd->write_ptr = &new_m[i];
		tapd->threads = &threads;
		tapd->op = 0; //SUB
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)(Threaded_ArithOp::threaded_runop), (LPVOID)(&*tapd), 0, 0);
	}
	while (threads) {}

	Matrix r(new_m);
	return r;
#else
	std::vector<std::vector<NINT>> new_m{};
	new_m.resize(rows.size());

	for (int i = 0; i < nm.shape.rows; i++) {
		std::vector<NINT> a = rows[i];
		std::vector<NINT> b = nm.rows[i];

		for (int x = 0; x < a.size(); x++) {
			new_m[i].emplace_back((a[x] - b[x]));
		}
	}

	Matrix r(new_m);
	return r;
#endif
}


#ifdef _PP_USE_THREADS
typedef struct sing_op_passdata {
	std::vector<NINT>* ptr_data;
	NINT num;
	short op; //1=add,2=sub,3=mul
	int* threads;
};
void sing_op_run(LPVOID pd) {
	sing_op_passdata sopd = *(static_cast<sing_op_passdata*>(pd));
	NINT num = sopd.num;
	switch (sopd.op) {
	case 1://add
		std::for_each(sopd.ptr_data->begin(), sopd.ptr_data->end(), [num](NINT& val) {val += num; });
		break;
	case 2://sub
		std::for_each(sopd.ptr_data->begin(), sopd.ptr_data->end(), [num](NINT& val) {val -= num; });
		break;
	case 3: //mul
		std::for_each(sopd.ptr_data->begin(), sopd.ptr_data->end(), [num](NINT& val){val *= num; });
		break;
	}
	*sopd.threads -= 1;
}

#endif

Matrix Matrix::operator+(const NINT to_add) {
#ifdef _PP_USE_THREADS
	int threads = 0;
#endif
	for (int i = 0; i < this->shape.rows; i++) {
#ifdef _PP_USE_THREADS
		sing_op_passdata* sopd = new sing_op_passdata;
		sopd->num = to_add;
		sopd->ptr_data = &rows[i];
		sopd->op = 1;
		sopd->threads = &threads;
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)sing_op_run, (LPVOID)(&*sopd), 0, 0);
#else
		for (int x = 0; x < rows[i].size(); x++) {
			rows[i][x] += to_add;
		}
#endif
	}
#ifdef _PP_USE_THREADS
	while(threads){}
#endif

	return *this;
}

Matrix Matrix::operator-(const NINT to_sub) {
#ifdef _PP_USE_THREADS
	int threads = 0;
#endif
	for (int i = 0; i < this->shape.rows; i++) {
#ifdef _PP_USE_THREADS
		sing_op_passdata* sopd = new sing_op_passdata;
		sopd->num = to_sub;
		sopd->ptr_data = &rows[i];
		sopd->op = 2;
		sopd->threads = &threads;
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)sing_op_run, (LPVOID)(&*sopd), 0, 0);
#else
		for (int x = 0; x < rows[i].size(); x++) {
			rows[i][x] *= to_sub;
		}
#endif
	}
#ifdef _PP_USE_THREADS
	while (threads) {}
#endif

	return *this;
} //sub 2 matrices
Matrix Matrix::operator*(const NINT to_mul) {
#ifdef _PP_USE_THREADS
	int threads = 0;
#endif
	for (int i = 0; i < this->shape.rows; i++) {
#ifdef _PP_USE_THREADS
		sing_op_passdata* sopd = new sing_op_passdata;
		sopd->num = to_mul;
		sopd->ptr_data = &rows[i];
		sopd->op = 3;
		sopd->threads = &threads;
		threads++;
		CreateThread(0, 64, (LPTHREAD_START_ROUTINE)sing_op_run, (LPVOID)(&*sopd), 0, 0);
#else
		for (int x = 0; x < rows[i].size(); x++) {
			rows[i][x] *= to_mul;
		}
#endif
	}
#ifdef _PP_USE_THREADS
	while (threads) {}
#endif

	return *this;
} //multiply 2 matrices

namespace Determinant_Simple_Calculators {

	/* 2x2 calculator */
	[[deprecated("Use .determinant() on a matrix class.")]]
	static NINT __x22(std::vector<std::vector<NINT>> vec) 
	{
		//unsafe

		NINT ad = vec[0][0] * vec[1][1];
		NINT bc = vec[0][1] * vec[1][0];
		return (ad - bc);
	}
	[[deprecated("Use .determinant() on a matrix class.")]]
	static NINT x22(Matrix m)  //2x2
	{
		if(m.get_rows() != 2 ||
			m.get_cols() != 2
			) {
			return 0;
		}

		//return (__x22(m.grab_matrix()));
	}

	/* 3x3 calculator */
	[[deprecated("Use .determinant() on a matrix class.")]]
	static NINT __x33(std::vector<std::vector<NINT>> vec) {

	}

	[[deprecated("Use .determinant() on a matrix class.")]]
	static NINT x33(Matrix m) {
		if(m.get_rows()!=3 ||
			m.get_cols()!= 3
			) {
			return 0;
		}

	//	return __x33(m.grab_matrix());
	}

	static NINT inversion_count(std::vector<int> permute) { //count the number of inversions a specific permutation has
		/* this works by, for each number, it counts how many numbers to the right of it are smaller.
		
		For example:
		lets say we have 
		{2,1,3}
		_^ We are on index 0, which is 2.
		1 is smaller than 2, so we +1 to our total inversion count. 3 is bigger, so we do nothing.
		*/
		NINT count = 0;
		for (int i = 0; i < permute.size(); i++) {
			NINT num = permute[i];
			for (int x = i; x < permute.size(); x++) {
				if (permute[x] < num) {
					count++;
				}
			}
		}
		return count;
	}

	static bool sgn(NINT val) {
		int rnd = round(val);
		return rnd % 2;
	}
}

typedef struct Permute {
	std::vector<int> set{};
	bool sign; //0=pos, 1=neg
};


NINT Matrix::determinant() {
	if (!is_square()) { return 0; }

	std::vector<Permute> permutes{};

	std::vector<int> indexes{};
	for (int i = 0; i < shape.rows; i++) {
		indexes.push_back(i + 1);
	}

	do {
		Permute pm;
		pm.set = indexes;
		pm.sign = Determinant_Simple_Calculators::sgn(Determinant_Simple_Calculators::inversion_count(indexes));
		permutes.push_back(pm);
	}while(std::next_permutation(indexes.begin(), indexes.end()));

	NINT determinant = 0;
	for (Permute pm : permutes) {

		//sub 1 from every index!!
		NINT indexed_product = 1;
		/* If my math is right, there will be 1 row for every element in the permute. */

		for (int i = 0; i < pm.set.size(); i++) {
			size_t idx = (pm.set[i]-1);
			indexed_product *= (NINT)(this->rows[i][idx]); //C-Style casting!!!!!!!
		}

		if (pm.sign) {
			indexed_product *= -1;
		}
		determinant += indexed_product;
	}
	return determinant;

}

NINT Matrix::tr() {
	return trace(*this);
}
NINT trace(Matrix m) {
	if (!m.is_square()) { return 0; }

	std::vector<std::vector<NINT>> vec = m.grab_matrix();
	NINT total = 0;
	for (int i = 0; i < vec.size(); i++) {
		total += vec[i][i];
	}
	return total;
}
