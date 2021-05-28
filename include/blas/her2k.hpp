// Copyright (c) 2017-2021, University of Tennessee. All rights reserved.
// Copyright (c) 2021, University of Colorado Denver. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// This program is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef BLAS_HER2K_HH
#define BLAS_HER2K_HH

#include "blas/utils.hpp"
#include "blas/syr2k.hpp"

namespace blas {

/**
 * Hermitian rank-k update:
 * \[
 *     C = \alpha A B^H + conj(\alpha) B A^H + \beta C,
 * \]
 * or
 * \[
 *     C = \alpha A^H B + conj(\alpha) B^H A + \beta C,
 * \]
 * where alpha and beta are scalars, C is an n-by-n Hermitian matrix,
 * and A and B are n-by-k or k-by-n matrices.
 *
 * Generic implementation for arbitrary data types.
 *
 * @param[in] layout
 *     Matrix storage, Layout::ColMajor or Layout::RowMajor.
 *
 * @param[in] uplo
 *     What part of the matrix C is referenced,
 *     the opposite triangle being assumed from symmetry:
 *     - Uplo::Lower: only the lower triangular part of C is referenced.
 *     - Uplo::Upper: only the upper triangular part of C is referenced.
 *
 * @param[in] trans
 *     The operation to be performed:
 *     - Op::NoTrans:   $C = \alpha A B^H + conj(\alpha) A^H B + \beta C$.
 *     - Op::ConjTrans: $C = \alpha A^H B + conj(\alpha) B A^H + \beta C$.
 *     - In the real    case, Op::Trans is interpreted as Op::ConjTrans.
 *       In the complex case, Op::Trans is illegal (see @ref syr2k instead).
 *
 * @param[in] n
 *     Number of rows and columns of the matrix C. n >= 0.
 *
 * @param[in] k
 *     - If trans = NoTrans: number of columns of the matrix A. k >= 0.
 *     - Otherwise:          number of rows    of the matrix A. k >= 0.
 *
 * @param[in] alpha
 *     Scalar alpha. If alpha is zero, A and B are not accessed.
 *
 * @param[in] A
 *     - If trans = NoTrans:
 *       the n-by-k matrix A, stored in an lda-by-k array [RowMajor: n-by-lda].
 *     - Otherwise:
 *       the k-by-n matrix A, stored in an lda-by-n array [RowMajor: k-by-lda].
 *
 * @param[in] lda
 *     Leading dimension of A.
 *     - If trans = NoTrans: lda >= max(1, n) [RowMajor: lda >= max(1, k)],
 *     - Otherwise:          lda >= max(1, k) [RowMajor: lda >= max(1, n)].
 *
 * @param[in] B
 *     - If trans = NoTrans:
 *       the n-by-k matrix B, stored in an ldb-by-k array [RowMajor: n-by-ldb].
 *     - Otherwise:
 *       the k-by-n matrix B, stored in an ldb-by-n array [RowMajor: k-by-ldb].
 *
 * @param[in] ldb
 *     Leading dimension of B.
 *     - If trans = NoTrans: ldb >= max(1, n) [RowMajor: ldb >= max(1, k)],
 *     - Otherwise:          ldb >= max(1, k) [RowMajor: ldb >= max(1, n)].
 *
 * @param[in] beta
 *     Scalar beta. If beta is zero, C need not be set on input.
 *
 * @param[in] C
 *     The n-by-n Hermitian matrix C,
 *     stored in an lda-by-n array [RowMajor: n-by-lda].
 *
 * @param[in] ldc
 *     Leading dimension of C. ldc >= max(1, n).
 *
 * @ingroup her2k
 */
template< typename TA, typename TB, typename TC >
void her2k(
    blas::Layout layout,
    blas::Uplo uplo,
    blas::Op trans,
    blas::size_t n, blas::size_t k,
    scalar_type<TA, TB, TC> alpha,  // note: complex
    TA const *A, blas::size_t lda,
    TB const *B, blas::size_t ldb,
    real_type<TA, TB, TC> beta,  // note: real
    TC       *C, blas::size_t ldc )
{
    typedef blas::scalar_type<TA, TB, TC> scalar_t;

    #define A(i_, j_) A[ (i_) + (j_)*lda ]
    #define B(i_, j_) B[ (i_) + (j_)*ldb ]
    #define C(i_, j_) C[ (i_) + (j_)*ldc ]

    // constants
    const scalar_t zero( 0.0 );
    const scalar_t one( 1.0 );

    // check arguments
    blas_error_if( layout != Layout::ColMajor &&
                   layout != Layout::RowMajor );
    blas_error_if( uplo != Uplo::Lower &&
                   uplo != Uplo::Upper &&
                   uplo != Uplo::General );
    blas_error_if( n < 0 );
    blas_error_if( k < 0 );

    // check and interpret argument trans
    // if (trans == Op::Trans) {
    //     blas_error_if_msg(
    //             (typeid(TA) != typeid(blas::real_type<TA>) ||
    //              typeid(TB) != typeid(blas::real_type<TB>)),
    //             "trans == Op::Trans && "
    //             "(typeid(TA) != typeid(blas::real_type<TA>) || "
    //             "typeid(TB) != typeid(blas::real_type<TB>))" );
    //     trans = Op::ConjTrans;
    // }
    // else {
        blas_error_if( trans != Op::NoTrans &&
                       trans != Op::ConjTrans );
    // }

    // adapt if row major
    if (layout == Layout::RowMajor) {

        // check lda and ldb
        blas_error_if( lda < ((trans == Op::NoTrans) ? k : n) );
        blas_error_if( ldb < ((trans == Op::NoTrans) ? k : n) );
        
        if (uplo == Uplo::Lower)
            uplo = Uplo::Upper;
        else if (uplo == Uplo::Upper)
            uplo = Uplo::Lower;
        trans = (trans == Op::NoTrans)
            ? Op::ConjTrans
            : Op::NoTrans;
        alpha = conj(alpha);
    }
    else {
        // check lda and ldb
        blas_error_if( lda < ((trans == Op::NoTrans) ? n : k) );
        blas_error_if( ldb < ((trans == Op::NoTrans) ? n : k) );
    }

    // check ldc
    blas_error_if( ldc < n );

    // quick return
    if (n == 0)
        return;

    // alpha == zero
    if (alpha == zero) {
        if (beta == zero) {
            if (uplo != Uplo::Upper) {
                for(size_t j = 0; j < n; ++j) {
                    for(size_t i = 0; i <= j; ++i)
                        C(i,j) = zero;
                }
            }
            else if (uplo != Uplo::Lower) {
                for(size_t j = 0; j < n; ++j) {
                    for(size_t i = j; i < n; ++i)
                        C(i,j) = zero;
                }
            }
            else {
                for(size_t j = 0; j < n; ++j) {
                    for(size_t i = 0; i < n; ++i)
                        C(i,j) = zero;
                }
            }
        } else if (beta != one) {
            if (uplo != Uplo::Upper) {
                for(size_t j = 0; j < n; ++j) {
                    for(size_t i = 0; i < j; ++i)
                        C(i,j) *= beta;
                    C(j,j) = beta * real( C(j,j) );
                }
            }
            else if (uplo != Uplo::Lower) {
                for(size_t j = 0; j < n; ++j) {
                    C(j,j) = beta * real( C(j,j) );
                    for(size_t i = j+1; i < n; ++i)
                        C(i,j) *= beta;
                }
            }
            else {
                for(size_t j = 0; j < n; ++j) {
                    for(size_t i = 0; i < j; ++i)
                        C(i,j) *= beta;
                    C(j,j) = beta * real( C(j,j) );
                    for(size_t i = j+1; i < n; ++i)
                        C(i,j) *= beta;
                }
            }
        }
        return;
    }

    // alpha != zero
    if (trans == Op::NoTrans) {
        if (uplo != Uplo::Lower) {
        // uplo == Uplo::Upper or uplo == Uplo::General
            for(size_t j = 0; j < n; ++j) {

                for(size_t i = 0; i < j; ++i)
                    C(i,j) *= beta;
                C(j,j) = beta * real( C(j,j) );

                for(size_t l = 0; l < k; ++l) {

                    scalar_t alphaConjBjl = alpha*conj( B(j,l) );
                    scalar_t conjAlphaAjl = conj( alpha*A(j,l) );

                    for(size_t i = 0; i < j; ++i) {
                        C(i,j) += A(i,l)*alphaConjBjl
                                + B(i,l)*conjAlphaAjl;
                    }
                    C(j,j) += 2 * real( A(j,l) * alphaConjBjl );
                }
            }
        }
        else { // uplo == Uplo::Lower
            for(size_t j = 0; j < n; ++j) {

                C(j,j) = beta * real( C(j,j) );
                for(size_t i = j+1; i < n; ++i)
                    C(i,j) *= beta;

                for(size_t l = 0; l < k; ++l) {

                    scalar_t alphaConjBjl = alpha*conj( B(j,l) );
                    scalar_t conjAlphaAjl = conj( alpha*A(j,l) );

                    C(j,j) += 2 * real( A(j,l) * alphaConjBjl );
                    for(size_t i = j+1; i < n; ++i) {
                        C(i,j) += A(i,l) * alphaConjBjl
                                + B(i,l) * conjAlphaAjl;
                    }
                }
            }
        }
    }
    else { // trans == Op::ConjTrans
        if (uplo != Uplo::Lower) {
        // uplo == Uplo::Upper or uplo == Uplo::General
            for(size_t j = 0; j < n; ++j) {
                for(size_t i = 0; i <= j; ++i) {

                    scalar_t sum1 = zero;
                    scalar_t sum2 = zero;
                    for(size_t l = 0; l < k; ++l) {
                        sum1 += conj( A(l,i) ) * B(l,j);
                        sum2 += conj( B(l,i) ) * A(l,j);
                    }

                    C(i,j) = (i < j)
                        ? alpha*sum1 + conj(alpha)*sum2 + beta*C(i,j)
                        : real( alpha*sum1 + conj(alpha)*sum2 )
                            + beta*real( C(i,j) );
                }

            }
        }
        else {
            // uplo == Uplo::Lower
            for(size_t j = 0; j < n; ++j) {
                for(size_t i = j; i < n; ++i) {

                    scalar_t sum1 = zero;
                    scalar_t sum2 = zero;
                    for(size_t l = 0; l < k; ++l) {
                        sum1 += conj( A(l,i) ) * B(l,j);
                        sum2 += conj( B(l,i) ) * A(l,j);
                    }

                    C(i,j) = (i > j)
                        ? alpha*sum1 + conj(alpha)*sum2 + beta*C(i,j)
                        : real( alpha*sum1 + conj(alpha)*sum2 )
                            + beta*real( C(i,j) );
                }

            }
        }
    }

    if (uplo == Uplo::General) {
        for(size_t j = 0; j < n; ++j) {
            for(size_t i = j+1; i < n; ++i)
                C(i,j) = conj( C(j,i) );
        }
    }

    #undef A
    #undef B
    #undef C
}

}  // namespace blas

#endif        //  #ifndef BLAS_HER2K_HH
