#include </home/crvena/anaconda3/envs/numc/include/python3.6m/Python.h>

typedef struct matrix
{
  int rows;     // number of rows
  int cols;     // number of columns
  double *data; // points to the begining of the data, row-major
  int is_1d;    // Whether this matrix is a 1d matrix
  // For 1D matrix, shape is (rows * cols)
  int ref_cnt;
  struct matrix *parent;
} matrix;

void rand_matrix (matrix *result, unsigned int seed, double low, double high);
int allocate_matrix (matrix **mat, int rows, int cols);
int allocate_matrix_ref (matrix **mat, matrix *from, int row_offset,
                         int col_offset, int rows, int cols);
void deallocate_matrix (matrix *mat);
double get (matrix *mat, int row, int col);
void set (matrix *mat, int row, int col, double val);
void fill_matrix (matrix *mat, double val);
int add_matrix (matrix *result, matrix *mat1, matrix *mat2);
int sub_matrix (matrix *result, matrix *mat1, matrix *mat2);
int mul_matrix (matrix *result, matrix *mat1, matrix *mat2)
    __attribute__ ((unused));
int pow_matrix (matrix *result, matrix *mat, int pow) __attribute__ ((unused));
int neg_matrix (matrix *result, matrix *mat);
int abs_matrix (matrix *result, matrix *mat);

int copy_matrix (matrix **mat, matrix *from);
int broadcast (matrix **to, matrix *from, int rows, int cols);
void print_matrix (matrix *mat);
