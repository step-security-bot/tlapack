/// @file MatrixMarket.hpp
/// @brief MaxtrixMarket class and random generators.
/// @author Thijs Steel, KU Leuven, Belgium
/// @author Weslley S Pereira, University of Colorado Denver, USA
//
// Copyright (c) 2021-2023, University of Colorado Denver. All rights reserved.
//
// This file is part of <T>LAPACK.
// <T>LAPACK is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#ifndef TLAPACK_MATRIXMARKET_HH
#define TLAPACK_MATRIXMARKET_HH

#include <tlapack/base/utils.hpp>

namespace tlapack {

/**
 * @brief Random number generator.
 *
 * This is a simple random number generator that generates a sequence of
 * pseudo-random numbers using a linear congruential generator (LCG).
 *
 * The LCG is defined by the recurrence relation:
 *
 * \[
 *     X_{n+1} = (a X_n + c) \mod m
 * \]
 *
 * where $X_0$ is the seed, $a$ is the multiplier, $c$ is the
 * increment, and $m$ is the modulus.
 */
class rand_generator {
   private:
    const uint64_t a = 6364136223846793005;  ///< multiplier (64-bit)
    const uint64_t c = 1442695040888963407;  ///< increment (64-bit)
    uint64_t state = 1302;                   ///< seed (64-bit)

   public:
    static constexpr uint32_t min() noexcept { return 0; }
    static constexpr uint32_t max() noexcept { return UINT32_MAX; }
    constexpr void seed(uint64_t s) noexcept { state = s; }

    /// Generates a pseudo-random number.
    constexpr uint32_t operator()()
    {
        state = state * a + c;
        return state >> 32;
    }
};

/**
 * @brief Helper function to generate random numbers.
 * @param[in,out] gen Random number generator.
 */
template <typename T, enable_if_t<is_real<T>, bool> = true>
T rand_helper(rand_generator& gen)
{
    return T(static_cast<float>(gen()) / static_cast<float>(gen.max()));
}

/**
 * @overload T rand_helper(rand_generator& gen)
 */
template <typename T, enable_if_t<is_complex<T>, bool> = true>
T rand_helper(rand_generator& gen)
{
    using real_t = real_type<T>;
    real_t r1(static_cast<float>(gen()) / static_cast<float>(gen.max()));
    real_t r2(static_cast<float>(gen()) / static_cast<float>(gen.max()));
    return complex_type<real_t>(r1, r2);
}

/**
 * @brief Helper function to generate random numbers.
 */
template <typename T, enable_if_t<is_real<T>, bool> = true>
T rand_helper()
{
    return T(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
}

/**
 * @overload T rand_helper()
 */
template <typename T, enable_if_t<is_complex<T>, bool> = true>
T rand_helper()
{
    using real_t = real_type<T>;
    real_t r1(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    real_t r2(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    return complex_type<real_t>(r1, r2);
}

/**
 * @brief MatrixMarket class.
 *
 * This class provides methods to read matrices from files and to generate
 * random or structured matrices.
 */
struct MatrixMarket {
    /**
     * @brief Read a dense matrix from an input stream (file, stdin, etc).
     *
     * The data is read in column-major format.
     *
     * @param[out] A Matrix.
     * @param[in,out] is Input stream.
     */
    template <TLAPACK_MATRIX matrix_t>
    void colmajor_read(matrix_t& A, std::istream& is) const
    {
        using idx_t = size_type<matrix_t>;

        const idx_t m = nrows(A);
        const idx_t n = ncols(A);

        for (idx_t j = 0; j < n; ++j)
            for (idx_t i = 0; i < m; ++i)
                is >> A(i, j);
    }

    /**
     * @brief Generate a random dense matrix.
     *
     * @param[out] A Matrix.
     */
    template <TLAPACK_MATRIX matrix_t>
    void random(matrix_t& A)
    {
        using T = type_t<matrix_t>;
        using idx_t = size_type<matrix_t>;

        const idx_t m = nrows(A);
        const idx_t n = ncols(A);

        for (idx_t j = 0; j < n; ++j)
            for (idx_t i = 0; i < m; ++i)
                A(i, j) = rand_helper<T>(gen);
    }

    /**
     * @brief Generate an upper- or lower-triangular random matrix.
     *
     * Put a single garbage value float(0xCAFEBABE) in the opposite triangle.
     *
     * @param[in] uplo Upper or lower triangular.
     * @param[out] A Matrix.
     */
    template <TLAPACK_UPLO uplo_t, TLAPACK_MATRIX matrix_t>
    void random(uplo_t uplo, matrix_t& A)
    {
        using T = type_t<matrix_t>;
        using idx_t = size_type<matrix_t>;

        const idx_t m = nrows(A);
        const idx_t n = ncols(A);

        if (uplo == Uplo::Upper) {
            for (idx_t j = 0; j < n; ++j)
                for (idx_t i = 0; i < m; ++i)
                    if (i <= j)
                        A(i, j) = rand_helper<T>(gen);
                    else
                        A(i, j) = T(float(0xCAFEBABE));
        }
        else {
            for (idx_t j = 0; j < n; ++j)
                for (idx_t i = 0; i < m; ++i)
                    if (i >= j)
                        A(i, j) = rand_helper<T>(gen);
                    else
                        A(i, j) = T(float(0xCAFEBABE));
        }
    }

    /**
     * @brief Generate a random upper Hessenberg matrix.
     *
     * Put a single garbage value float(0xFA57C0DE) below the first subdiagonal.
     *
     * @param[out] A Matrix.
     */
    template <TLAPACK_MATRIX matrix_t>
    void hessenberg(matrix_t& A)
    {
        using T = type_t<matrix_t>;
        using idx_t = size_type<matrix_t>;

        const idx_t m = nrows(A);
        const idx_t n = ncols(A);

        for (idx_t j = 0; j < n; ++j)
            for (idx_t i = 0; i < m; ++i)
                if (i <= j + 1)
                    A(i, j) = rand_helper<T>(gen);
                else
                    A(i, j) = T(float(0xFA57C0DE));
    }

    /**
     * @brief Generate a Hilbert matrix.
     *
     * The Hilbert matrix is defined as:
     *
     * \[
     *    A_{ij} = \frac{1}{i + j + 1}
     * \]
     *
     * @param[out] A Matrix.
     */
    template <TLAPACK_MATRIX matrix_t>
    void hilbert(matrix_t& A) const
    {
        using T = type_t<matrix_t>;
        using idx_t = size_type<matrix_t>;

        const idx_t m = nrows(A);
        const idx_t n = ncols(A);

        for (idx_t j = 0; j < n; ++j)
            for (idx_t i = 0; i < m; ++i)
                A(i, j) = T(1) / T(i + j + 1);
    }

    /**
     * @brief Generate a matrix with a single value in all entries.
     *
     * @param[out] A Matrix.
     * @param[in] val Value.
     */
    template <TLAPACK_MATRIX matrix_t>
    void single_value(matrix_t& A, const type_t<matrix_t>& val) const
    {
        using idx_t = size_type<matrix_t>;

        const idx_t m = nrows(A);
        const idx_t n = ncols(A);

        for (idx_t j = 0; j < n; ++j)
            for (idx_t i = 0; i < m; ++i)
                A(i, j) = val;
    }

   private:
    rand_generator gen;
};

}  // namespace tlapack

#endif  // TLAPACK_MATRIXMARKET_HH