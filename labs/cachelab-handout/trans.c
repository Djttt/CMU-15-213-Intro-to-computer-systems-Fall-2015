/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void trans(int M, int N, int A[N][M], int B[M][N]);
void trans_with_blocking(int M, int N, int A[N][M], int B[M][N]);
void my_trans(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    my_trans(M, N, A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 


/**
 * b_row = 8, b_col = 8 for M, N = 32, miss = 344
 * 
 * b_row = 16, b_col = 12 for M, N = 67, 61, misses = 1997
 */
void trans_with_blocking(int M, int N, int A[N][M], int B[M][N]) {
    // block size = 8
    int b_row = 16;
    int b_col = 12;
    int i, j;
    int temp;
    for (i = 0; i < N; i+=b_row) {
        for (j = 0; j < M; j+=b_col) {
            for (int i1 = i; i1 < b_row + i && i1 < N; i1++) {
                for (int j1 = j; j1 < b_col + j && j1 < M; j1++) {
                    temp = A[i1][j1];
                    B[j1][i1] = temp;
                }
            }
        }
    }
}

/**
 * trans matrics with A's col to B's row
 */
char trans_col_desc[] = "my solution for trans.c";
void my_trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, ii, tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;

    // 32x32
    if (M == 32 && N == 32) {
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (ii = i; ii < i + 8; ii++) {
                    // 按行读，缓存充分利用
                    tmp0 = A[ii][j];
                    tmp1 = A[ii][j + 1];
                    tmp2 = A[ii][j + 2];
                    tmp3 = A[ii][j + 3];
                    tmp4 = A[ii][j + 4];
                    tmp5 = A[ii][j + 5];
                    tmp6 = A[ii][j + 6];
                    tmp7 = A[ii][j + 7];
                    
                    B[j][ii] = tmp0;
                    B[j + 1][ii] = tmp1;
                    B[j + 2][ii] = tmp2;
                    B[j + 3][ii] = tmp3;
                    B[j + 4][ii] = tmp4;
                    B[j + 5][ii] = tmp5;
                    B[j + 6][ii] = tmp6;
                    B[j + 7][ii] = tmp7;
                }
            }
        }
    } // 64x64
    else if (M == 64 && N == 64) {
        for (i = 0; i < N; i += 4) {
            for (j = 0; j < M; j += 4) {
                for (ii = i; ii < i + 4; ii++) {
                    // 按行读，缓存充分利用
                    tmp0 = A[ii][j];
                    tmp1 = A[ii][j + 1];
                    tmp2 = A[ii][j + 2];
                    tmp3 = A[ii][j + 3];
              
                    B[j][ii] = tmp0;
                    B[j + 1][ii] = tmp1;
                    B[j + 2][ii] = tmp2;
                    B[j + 3][ii] = tmp3;
                }
            }
        }
    }
    else {
        trans_with_blocking(M, N, A, B);
    }
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

