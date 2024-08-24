#include "simd.h"
#include <omp.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>

long long int sum(int vals[NUM_ELEMS]) {
  clock_t start = clock();

  long long int sum = 0;
  for (unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
    for (unsigned int i = 0; i < NUM_ELEMS; i++) {
      if (vals[i] >= 128) {
        sum += vals[i];
      }
    }
  }
  clock_t end = clock();
  printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
  return sum;
}

long long int sum_unrolled(int vals[NUM_ELEMS]) {
  clock_t start = clock();
  long long int sum = 0;

  for (unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
    for (unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
      if (vals[i] >= 128)
        sum += vals[i];
      if (vals[i + 1] >= 128)
        sum += vals[i + 1];
      if (vals[i + 2] >= 128)
        sum += vals[i + 2];
      if (vals[i + 3] >= 128)
        sum += vals[i + 3];
    }

    // This is what we call the TAIL CASE
    // For when NUM_ELEMS isn't a multiple of 4
    // NONTRIVIAL FACT: NUM_ELEMS / 4 * 4 is the largest multiple of 4 less than
    // NUM_ELEMS
    for (unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
      if (vals[i] >= 128) {
        sum += vals[i];
      }
    }
  }
  clock_t end = clock();
  printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
  return sum;
}

long long int sum_simd(int vals[NUM_ELEMS]) {
  clock_t start = clock();
  __m128i _127 = _mm_set1_epi32(
      127); // This is a vector with 127s in it... Why might you need this?
  long long int result = 0; // This is where you should put your final result!
  /* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */

  for (unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
    /* YOUR CODE GOES HERE */
    __m128i xmm = _mm_setzero_si128();
    __m128i sum_vec = _mm_setzero_si128();
    __m128i mask = _mm_setzero_si128();
    for (unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
      xmm = _mm_loadu_si128((__m128i *)&vals[i]);
      mask = _mm_cmpgt_epi32(xmm, _127);
      xmm = _mm_and_si128(mask, xmm);
      sum_vec = _mm_add_epi32(xmm, sum_vec);
    }
    int sum_arr[4];
    _mm_storeu_si128((__m128i *)sum_arr, sum_vec);
    result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

    /* You'll need a tail case. */
    for (unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++)
      result += vals[i];
  }
  clock_t end = clock();
  printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
  return result;
}

long long int sum_simd_parallel(int vals[NUM_ELEMS]) {
  clock_t start = clock();
  long long int result = 0; // This is where you should put your final result!
  /* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */

  for (unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
    /* YOUR CODE GOES HERE */
    const int THREAD_NUM = 2;
    omp_set_num_threads(THREAD_NUM);
    int sum[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++)
      sum[i] = 0;
#pragma omp parallel
    {
      int id = omp_get_thread_num();
      __m128i _127 = _mm_set1_epi32(127);
      __m128i mask = _mm_setzero_si128();
      __m128i xmm = _mm_setzero_si128();
      __m128i sum_vec = _mm_setzero_si128();
      for (unsigned int i = id; i < NUM_ELEMS / 4; i += THREAD_NUM) {
        xmm = _mm_loadu_si128((__m128i *)&vals[i * 4]);
        mask = _mm_cmpgt_epi32(xmm, _127);
        xmm = _mm_and_si128(mask, xmm);
        sum_vec = _mm_add_epi32(xmm, sum_vec);
      }
      int sum_arr[4];
      _mm_storeu_si128((__m128i *)sum_arr, sum_vec);
      sum[id] = sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];
#pragma omp critical
      result += sum[id];
    }
    /* You'll need a tail case. */
    for (unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++)
      result += vals[i];
  }
  clock_t end = clock();
  printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
  return result;
}

long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
  clock_t start = clock();
  __m128i _127 = _mm_set1_epi32(127);
  long long int result = 0;
  for (unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
    /* COPY AND PASTE YOUR sum_simd() HERE */
    /* MODIFY IT BY UNROLLING IT */
    __m128i xmm_1 = _mm_setzero_si128();
    __m128i xmm_2 = _mm_setzero_si128();
    __m128i xmm_3 = _mm_setzero_si128();
    __m128i xmm_4 = _mm_setzero_si128();
    __m128i mask_1 = _mm_setzero_si128();
    __m128i mask_2 = _mm_setzero_si128();
    __m128i mask_3 = _mm_setzero_si128();
    __m128i mask_4 = _mm_setzero_si128();
    __m128i sum_vec = _mm_setzero_si128();
    for (unsigned int i = 0; i < NUM_ELEMS / 16 * 16; i += 16) {
      xmm_1 = _mm_loadu_si128((__m128i *)&vals[i]);
      xmm_2 = _mm_loadu_si128((__m128i *)&vals[i + 4]);
      xmm_3 = _mm_loadu_si128((__m128i *)&vals[i + 8]);
      xmm_4 = _mm_loadu_si128((__m128i *)&vals[i + 12]);
      mask_1 = _mm_cmpgt_epi32(xmm_1, _127);
      mask_2 = _mm_cmpgt_epi32(xmm_2, _127);
      mask_3 = _mm_cmpgt_epi32(xmm_3, _127);
      mask_4 = _mm_cmpgt_epi32(xmm_4, _127);
      xmm_1 = _mm_and_si128(mask_1, xmm_1);
      xmm_2 = _mm_and_si128(mask_2, xmm_2);
      xmm_3 = _mm_and_si128(mask_3, xmm_3);
      xmm_4 = _mm_and_si128(mask_4, xmm_4);
      sum_vec = _mm_add_epi32(xmm_1, sum_vec);
      sum_vec = _mm_add_epi32(xmm_2, sum_vec);
      sum_vec = _mm_add_epi32(xmm_3, sum_vec);
      sum_vec = _mm_add_epi32(xmm_4, sum_vec);
    }

    /* You'll need 1 or maybe 2 tail cases here. */
    for (unsigned int i = NUM_ELEMS / 16 * 16; i < NUM_ELEMS / 4 * 4; i += 4) {
      xmm_1 = _mm_loadu_si128((__m128i *)&vals[i]);
      mask_1 = _mm_cmpgt_epi32(xmm_1, _127);
      xmm_1 = _mm_and_si128(mask_1, xmm_1);
      sum_vec = _mm_add_epi32(xmm_1, sum_vec);
    }
    int sum_arr[4];
    _mm_storeu_si128((__m128i *)sum_arr, sum_vec);
    result += sum_arr[0] + sum_arr[1] + sum_arr[2] + sum_arr[3];

    for (unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++)
      result += vals[i];
  }
  clock_t end = clock();
  printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
  return result;
}
