/// @file gelqf.hpp
/// @author Yuxin Cai, University of Colorado Denver, USA
/// Adapted from @see https://github.com/Reference-LAPACK/lapack/blob/master/SRC/zgelqf.f
//
// Copyright (c) 2014-2022, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef __TLAPACK_GELQF_HH__
#define __TLAPACK_GELQF_HH__

#include "base/utils.hpp"
#include "base/types.hpp"
#include "lapack/larfg.hpp"
#include "lapack/larf.hpp"

namespace tlapack
{
    /** Computes an LQ factorization of a complex m-by-n matrix A using
     *  a blocked algorithm.
     *
     * The matrix Q is represented as a product of elementary reflectors.
     * \[
     *          Q = H(k)**H ... H(2)**H H(1)**H,
     * \]
     * where k = min(m,n). Each H(j) has the form
     * \[
     *          H(j) = I - tauw * w * w**H
     * \]
     * where tauw is a complex scalar, and w is a complex vector with
     * \[
     *          w[0] = w[1] = ... = w[j-1] = 0; w[j] = 1,
     * \]
     * with w[j+1]**H through w[n]**H is stored on exit in the jth row of A.
     * tauw is stored in TT(j,i), where 0 <= i < nb and i = j (mod nb).
     *
     * @return  0 if success
     *
     * @param[in,out] A m-by-n matrix.
     *      On exit, the elements on and below the diagonal of the array
     *      contain the m by min(m,n) lower trapezoidal matrix L (L is
     *      lower triangular if m <= n); the elements above the diagonal,
     *      with the array tauw, represent the unitary matrix Q as a
     *      product of elementary reflectors.
     * 
     * @param[in,out] TT m-by-nb matrix.
     *      In the representation of the block reflector.
     *      tauw[j] is stored in TT(j,i), where 0 <= i < nb and i = j (mod nb).
     *      On exit, TT( 0:k, 0:nb ) contains blocks used to build Q :
     *      \[
     *          Q^H
     *          =
     *          [ I - W(0:nb,0:k)^T * TT(0:nb,0:nb) * conj(W(0:nb,0:k)) ]
     *          *
     *          [ I - W(nb:2nb,0:k)^T * TT(nb:2nb,0:nb) * conj(W(nb:2nb,0:k)) ]
     *          *
     *          ...
     *      \]
     *
     * @param work Vector of size m.
     * 
     * @param nb Constant of block height.
     *
     * @ingroup gelqf
     */
    template <typename matrix_t, class work_t>
    int gelqf(matrix_t &A, matrix_t &TT, work_t &work, const size_type<matrix_t> &nb)
    {
        using idx_t = size_type<matrix_t>;
        using range = std::pair<idx_t, idx_t>;

        // constants
        const idx_t m = nrows(A);
        const idx_t n = ncols(A);
        const idx_t k = min(m, n);

        // check arguments
        tlapack_check_false(access_denied(dense, write_policy(A)), -1);
        tlapack_check_false( nrows(TT) < m || ncols(TT) < nb, -2);
        tlapack_check_false((idx_t)size(work) < m, -3);

        for (idx_t j = 0; j < k; j += nb)
        {
            // Use blocked code initially
            idx_t ib = std::min<idx_t>(nb, k - j);

            // Compute the LQ factorization of the current block A(j:j+ib-1,j:n)
            auto TT1 = slice(TT, range(j, j + ib), range(0, ib));
            auto A11 = slice(A, range(j, j + ib), range(j, n));
            auto tauw1 = diag(TT1);

            gelq2(A11, tauw1, work);

            if (j + ib < k)
            {
                // Form the triangular factor of the block reflector H = H(j) H(j+1) . . . H(j+ib-1)
                larft(Direction::Forward, StoreV::Rowwise, A11, tauw1, TT1);

                // Apply H to A(j+ib:m,j:n) from the right
                auto A12 = slice(A, range(j + ib, m), range(j, n));
                auto work1 = slice(TT, range(j + ib, m), range(0, ib));

                larfb(
                    Side::Right,
                    Op::NoTrans,
                    Direction::Forward,
                    StoreV::Rowwise,
                    A11, TT1, A12, work1);
            }
        }

        return 0;
    }
}
#endif // __TLAPACK_GELQF_HH__
