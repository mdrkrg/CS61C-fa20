#include "matrix.h"
#include <omp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
 */

/* Assume mats are non-NULL */
int
size_mismatch (matrix *mat_1, matrix *mat_2)
{
  if (mat_1->rows != mat_2->rows || mat_1->cols != mat_2->cols)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

/*
 * Generates a random double between `low` and `high`.
 */
double
rand_double (double low, double high)
{
  double range = (high - low);
  double div = RAND_MAX / range;
  return low + (rand () / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void
rand_matrix (matrix *result, unsigned int seed, double low, double high)
{
  srand (seed);
  for (int i = 0; i < result->rows; i++)
    {
      for (int j = 0; j < result->cols; j++)
        {
          set (result, i, j, rand_double (low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data
 * array and initialize all entries to be zeros. Remember to set all fieds of
 * the matrix struct. `parent` should be set to NULL to indicate that this
 * matrix is not a slice. You should return -1 if either `rows` or `cols` or
 * both have invalid values, or if any call to allocate memory in this function
 * fails. If you don't set python error messages here upon failure, then
 * remember to set it in numc.c. Return 0 upon success and non-zero upon
 * failure.
 */
int
allocate_matrix (matrix **mat, int rows, int cols)
{
  if (rows < 1 || cols < 1)
    {
      return -1;
    }
  if ((*mat = malloc (sizeof (matrix))) == NULL)
    {
      return -2;
    }
  (*mat)->rows = rows;
  (*mat)->cols = cols;
  (*mat)->is_1d = (rows == 1 || cols == 1) ? 1 : 0;
  (*mat)->data = calloc (rows * cols, sizeof (double)); // calloc zero inits
  if ((*mat)->data == NULL)
    {
      return -2;
    }
  (*mat)->parent = NULL;
  (*mat)->ref_cnt = 1;
  return 0;
}

int
copy_matrix (matrix **mat, matrix *from)
{
  double *p_to, *p_from, *range;
  size_t i, j;
  size_t from_stride;
  int rows, cols;
  if (from == NULL)
    {
      return -3;
    }
  if ((*mat = malloc (sizeof (matrix))) == NULL)
    {
      return -2;
    }
  (*mat)->rows = rows = from->rows;
  (*mat)->cols = cols = from->cols;
  (*mat)->is_1d = from->is_1d;
  (*mat)->parent = NULL;
  (*mat)->ref_cnt = 1;
  if (((*mat)->data = calloc (rows * cols, sizeof (double))) == NULL)
    {
      return -2;
    }
  p_to = (*mat)->data;
  p_from = from->data;
  if (from->parent)
    {
      from_stride = from->parent->cols;
      for (i = 0; i < rows; i++, p_from += from_stride)
        {
          for (j = 0; j < cols; j++, p_to++)
            {
              *p_to = p_from[j];
            }
        }
    }
  else
    {
      range = (*mat)->data + rows * cols;
      for (; p_to < range; p_to++, p_from++)
        {
          *p_to = *p_from;
        }
    }
  return 0;
}

int
flatten_matrix (matrix **to, matrix *from)
{
  int retval;
  if (!from)
    {
      return -3;
    }
  if (!to)
    {
      return -3;
    }
  retval = 0;
  if (from->parent)
    {
      retval = copy_matrix (to, from);
      if (retval)
        {
          return retval;
        }
    }
  else
    {
      *to = from;
    }
  return retval;
}

void
print_matrix (matrix *mat)
{
  if (!mat)
    {
      return;
    }
  if (!mat->parent)
    {
      for (int i = 0; i < mat->rows; i++)
        {
          for (int j = 0; j < mat->cols; j++)
            {
              printf ("%f ", mat->data[j + i * mat->cols]);
            }
          printf ("\n");
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and
 * `cols` columns. This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to
 * set it in numc.c. Return 0 upon success and non-zero upon failure.
 *
 * Plain structure of parent-children:
 * - Only parent own all the data
 * - Children are always leaves
 */
int
allocate_matrix_ref (matrix **mat, matrix *from, int row_offset,
                     int col_offset, int rows, int cols)
{
  size_t from_stride;
  matrix *root_mat;
  if (from == NULL || mat == NULL)
    {
      return -3;
    }
  if (rows < 1 || cols < 1 || row_offset < 0 || col_offset < 0
      || row_offset + rows > from->rows || col_offset + cols > from->cols)
    {
      return -1;
    }
  root_mat = (from->parent == NULL) ? from : from->parent;
  *mat = malloc (sizeof (matrix));
  if (*mat == NULL)
    {
      return -2;
    }
  (*mat)->rows = rows;
  (*mat)->cols = cols;
  root_mat->ref_cnt++;
  (*mat)->ref_cnt = 1;
  from_stride = (from->parent) ? from->parent->cols : from->cols;
  (*mat)->data = from->data + row_offset * from_stride + col_offset;
  // If we assign parent like this, we will get a plain structure.
  (*mat)->parent = (from->parent == NULL) ? from : from->parent;
  (*mat)->is_1d = (rows == 1 || cols == 1) ? 1 : 0;
  return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix
 * loses all of its reference pointers. You need to make sure that you only
 * free `mat->data` if no other existing matrices are also referring this data
 * array. See the spec for more information.
 */
void
deallocate_matrix (matrix *mat)
{
  if (mat == NULL)
    {
      return;
    }
  if (mat->parent == NULL)
    {
      mat->ref_cnt--;
      if (mat->ref_cnt == 0)
        {
          free (mat->data);
          free (mat);
        }
    }
  else
    {
      deallocate_matrix (mat->parent);
      free (mat);
    }
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double
get (matrix *mat, int row, int col)
{
  if (mat->parent == NULL)
    {
      return mat->data[row * mat->cols + col];
    }
  else
    {
      return mat->data[row * mat->parent->cols + col];
    }
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void
set (matrix *mat, int row, int col, double val)
{
  if (mat->parent == NULL)
    {
      mat->data[row * mat->cols + col] = val;
    }
  else
    {
      mat->data[row * mat->parent->cols + col] = val;
    }
}

/*
 * Set all entries in mat to val
 */
void
fill_matrix (matrix *mat, double val)
{
  double *p, *row_start;
  size_t stride;
  const double *range;
  if (mat->parent == NULL)
    {
      range = mat->data + mat->rows * mat->cols;
      for (p = mat->data + 0; p < range; p++)
        {
          *p = val;
        }
    }
  else
    {
      stride = mat->parent->cols;
      for (int r = 0; r < mat->rows; r++)
        {
          row_start = mat->data + r * stride;
          for (p = row_start; p < row_start + mat->cols; p++)
            {
              *p = val;
            }
        }
    }
}

int
broadcast (matrix **to, matrix *from, int rows, int cols)
{
  int retval, size;
  int row_1d, col_1d;
  size_t i, j;
  size_t from_stride;
  register double *p_to, *p_from;
  if (from == NULL)
    {
      return -3;
    }
  size = rows * cols;
  row_1d = from->rows == 1;
  col_1d = from->cols == 1;
  if (from->rows > rows || from->cols > cols || (!row_1d && !col_1d))
    {
      return -1;
    }
  if ((retval = allocate_matrix (to, rows, cols)))
    {
      return retval;
    }

  p_to = (*to)->data;
  p_from = from->data;
  if (row_1d && col_1d)
    {
      // Fill matrix?
      for (; p_to < (*to)->data + size; p_to++)
        {
          *p_to = *p_from;
        }
    }
  else if (row_1d)
    {
      for (i = 0; i < rows; i++, p_from = from->data)
        {
          for (j = 0; j < cols; j++, p_to++, p_from++)
            {
              *p_to = *p_from;
            }
        }
    }
  else
    {
      from_stride = (from->parent) ? from->parent->cols : from->cols;
      for (i = 0; i < rows; i++, p_from += from_stride)
        {
          for (j = 0; j < cols; j++, p_to++)
            {
              *p_to = *p_from;
            }
        }
    }
  return 0;
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int
add_matrix (matrix *result, matrix *mat1, matrix *mat2)
{
  int row_eq, col_eq; // bool
  int row_gt, col_gt; // bool
  int rows, cols;
  int retval;
  size_t i, j;
  // size_t mat1_stride, mat2_stride;
  // double *p_result, *p_mat1, *p_mat2;
  double single_val;
  // double *range;
  matrix *smaller_mat, *larger_mat;
  matrix *tmp_1, *tmp_2;
  row_eq = mat1->rows == mat2->rows;
  col_eq = mat1->cols == mat2->cols;
  row_gt = mat1->rows > mat2->rows;
  col_gt = mat1->cols > mat2->cols;
  if (!col_eq && (mat1->cols != 1 && mat2->cols != 1))
    {
      return -1;
    }
  if (!row_eq && (mat1->rows != 1 && mat2->rows != 1))
    {
      return -1;
    }
  if (!result)
    {
      return -3;
    }
  /* Only these cases:
   * bigger one:  m | n
   * smaller one: m | n
   *              m | 1
   *              1 | n
   *              1 | 1
   */
  rows = row_gt ? mat1->rows : mat2->rows;
  cols = col_gt ? mat1->cols : mat2->cols;
  // p_mat1 = mat1->data;
  // p_mat2 = mat2->data;
  tmp_1 = tmp_2 = NULL;
  if (row_eq && col_eq)
    {
      retval = flatten_matrix (&tmp_1, mat1);
      if (retval)
        {
          return retval;
        }
      retval = flatten_matrix (&tmp_2, mat2);
      if (retval)
        {
          return retval;
        }
      for (int i = 0; i < rows * cols; i++)
        {
          result->data[i] = tmp_1->data[i] + tmp_2->data[i];
        }
    }
  else
    {
      if (row_gt || col_gt)
        {
          larger_mat = mat1;
          smaller_mat = mat2;
        }
      else
        {
          larger_mat = mat2;
          smaller_mat = mat1;
          row_gt = larger_mat->rows > smaller_mat->rows;
          col_gt = larger_mat->cols > smaller_mat->cols;
        }

      if (row_gt && col_gt)
        { // smaller one is single value
          retval = flatten_matrix (&tmp_1, larger_mat);
          if (retval)
            {
              return retval;
            }
          single_val = *(smaller_mat->data);
          for (i = 0; i < rows * cols; i++)
            {
              result->data[i] = tmp_1->data[i] + single_val;
            }
        }
      else
        {
          retval = broadcast (&tmp_1, smaller_mat, rows, cols);
          if (retval)
            {
              return retval;
            }
          retval = flatten_matrix (&tmp_2, larger_mat);
          if (retval)
            {
              return retval;
            }
          for (i = 0; i < rows * cols; i++)
            {
              result->data[i] = tmp_1->data[i] + tmp_2->data[i];
            }
        }
    }
  if (tmp_1 != mat1 && tmp_1 != mat2)
    deallocate_matrix (tmp_1);
  if (tmp_2 != mat1 && tmp_2 != mat2)
    deallocate_matrix (tmp_2);
  return 0;
}
//   if (row_eq && col_eq) {
//     if (!mat1->parent && !mat2->parent) {
//       if ((retval = allocate_matrix(&result, rows, cols))) {
//         return retval;
//       }
//       range = result->data + rows * cols;
//       for (p_result = result->data; p_result < range; p_result++, p_mat1++,
//       p_mat2++) {
//         *p_result = *p_mat1 + *p_mat2;
//       }
//     } else if (!mat1->parent) {
//       if ((retval = copy_matrix(&result, mat2))) {
//         return retval;
//       }
//       range = result->data + rows * cols;
//       for (p_result = result->data; p_result < range; p_result++, p_mat1++)
//       {
//         *p_result += *p_mat1;
//       }
//     } else if (!mat2->parent) {
//       if ((retval = copy_matrix(&result, mat1))) {
//         return retval;
//       }
//       range = result->data + rows * cols;
//       for (p_result = result->data; p_result < range; p_result++, p_mat2++)
//       {
//         *p_result += *p_mat2;
//       }
//     } else {
//       if ((retval = copy_matrix(&result, mat1))) {
//         return retval;
//       }
//       mat2_stride = mat2->parent->cols;
//       for (i = 0, p_result = result->data; i < rows; i++, p_mat2 +=
//       mat2_stride) {
//         for (j = 0; j < cols; j++, p_result++) {
//           *p_result += p_mat2[j];
//         }
//       }
//     }
//   } else {
//     if (row_gt || col_gt) {
//       larger_mat = mat1;
//       smaller_mat = mat2;
//     } else {
//       larger_mat = mat2;
//       smaller_mat = mat1;
//       row_gt = larger_mat->rows > smaller_mat->rows;
//       col_gt = larger_mat->cols > smaller_mat->cols;
//     }
//
//     if (row_gt && col_gt) { // smaller one is single value
//       if ((retval = copy_matrix(&result, larger_mat))) {
//         return retval;
//       }
//       range = result->data + rows * cols;
//       single_val = *(smaller_mat->data);
//       for (p_result = result->data; p_result < range; p_result++) {
//         *p_result += single_val;
//       }
//     } else {
//       if ((retval = broadcast(&result, smaller_mat, rows, cols))) {
//         return retval;
//       }
//       p_mat1 = larger_mat->data;
//       if (larger_mat->parent) {
//         mat1_stride = larger_mat->parent->cols;
//         for (i = 0, p_result = result->data; i < rows; i++, p_mat1 +=
//         mat1_stride) {
//           for (j = 0; j < cols; j++, p_result++) {
//             *p_result += p_mat1[j];
//           }
//         }
//       } else {
//         range = result->data + rows * cols;
//         for (p_result = result->data; p_result < range; p_result++,
//         p_mat1++)
//         {
//           *p_result += *p_mat1;
//         }
//       }
//     }
//   }
//   return 0;
// }

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int
sub_matrix (matrix *result, matrix *mat1, matrix *mat2)
{
  int retval;
  matrix *tmp;
  tmp = NULL;
  if (!result || !mat1 || !mat2)
    {
      return -3;
    }
  retval = allocate_matrix (&tmp, mat2->rows, mat2->cols);
  if (retval)
    {
      return retval;
    }
  retval = neg_matrix (tmp, mat2);
  if (retval)
    {
      return retval;
    }
  retval = add_matrix (result, mat1, tmp);
  deallocate_matrix (tmp);
  return retval;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying
 * individual elements.
 */
int
mul_matrix (matrix *result, matrix *mat1, matrix *mat2)
{
  /* TODO: YOUR CODE HERE */
  return 0;
}

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise
 * multiplication.
 */
int
pow_matrix (matrix *result, matrix *mat, int pow)
{
  /* TODO: YOUR CODE HERE */
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int
neg_matrix (matrix *result, matrix *mat)
{
  // matrix *tmp;
  int retval;
  int rows, cols;
  size_t i, j;
  size_t from_stride;
  if (!result || !mat)
    {
      return -3;
    }
  if (size_mismatch (mat, result))
    {
      return -1;
    }

  rows = mat->rows;
  cols = mat->cols;
  from_stride = (mat->parent) ? mat->parent->cols : mat->cols;

  for (i = 0; i < rows; i++)
    {
      for (j = 0; j < cols; j++)
        {
          result->data[i * cols + j] = -mat->data[i * from_stride + j];
        }
    }
  // tmp = NULL;
  // retval = flatten_matrix(&tmp, mat);
  // if (retval) { return retval; }
  //
  // for (i = 0; i < rows * cols; i++) {
  //   result->data[i] = -(tmp->data[i]);
  // }
  // deallocate_matrix(tmp);
  return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int
abs_matrix (matrix *result, matrix *mat)
{
  matrix *tmp;
  int retval;
  int rows, cols;
  double val;
  size_t i, j;
  size_t from_stride;
  if (!result || !mat)
    {
      return -3;
    }
  if (size_mismatch (mat, result))
    {
      return -1;
    }

  rows = mat->rows;
  cols = mat->cols;
  from_stride = (mat->parent) ? mat->parent->cols : mat->cols;
  // tmp = NULL;
  // // PERF: Can do better (because it's simple)
  // retval = flatten_matrix(&tmp, mat);
  // if (retval) {
  //   return retval;
  // }

  // for (i = 0; i < rows * cols; i++) {
  //   val = tmp->data[i];
  //   result->data[i] = (val > 0) ? val : -val;
  // }
  for (i = 0; i < rows; i++)
    {
      for (j = 0; j < cols; j++)
        {
          val = mat->data[i * from_stride + j];
          result->data[i * cols + j] = fabs (val);
        }
    }
  // deallocate_matrix(tmp);
  return 0;
}
