#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <iostream>
#include <random> // Requires C++ 11

#include <GenEigsSolver.h>
#include <MatOp/DenseGenMatProd.h>
#include <MatOp/SparseGenMatProd.h>

using namespace Spectra;

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

typedef Eigen::MatrixXd Matrix;
typedef Eigen::VectorXd Vector;
typedef Eigen::MatrixXcd ComplexMatrix;
typedef Eigen::VectorXcd ComplexVector;
typedef Eigen::SparseMatrix<double> SpMatrix;

// Traits to obtain operation type from matrix type
template <typename MatType>
struct OpTypeTrait
{
    typedef DenseGenMatProd<double> OpType;
};

template <>
struct OpTypeTrait<SpMatrix>
{
    typedef SparseGenMatProd<double> OpType;
};

// Generate random sparse matrix
SpMatrix sprand(int size, double prob = 0.5)
{
    SpMatrix mat(size, size);
    std::default_random_engine gen;
    gen.seed(0);
    std::uniform_real_distribution<double> distr(-1.0, 1.0);
    for(int i = 0; i < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            if(distr(gen) < prob)
                mat.insert(i, j) = distr(gen);
        }
    }
    return mat;
}

template <typename MatType, int SelectionRule>
void run_test(const MatType& mat, int k, int m)
{
    typename OpTypeTrait<MatType>::OpType op(mat);
    GenEigsSolver<double, SelectionRule, typename OpTypeTrait<MatType>::OpType>
        eigs(&op, k, m);
    eigs.init();
    int nconv = eigs.compute();
    int niter = eigs.num_iterations();
    int nops  = eigs.num_operations();

    INFO( "nconv = " << nconv );
    INFO( "niter = " << niter );
    INFO( "nops  = " << nops );
    REQUIRE( eigs.info() == SUCCESSFUL );

    ComplexVector evals = eigs.eigenvalues();
    ComplexMatrix evecs = eigs.eigenvectors();
    ComplexMatrix err = mat * evecs - evecs * evals.asDiagonal();

    INFO( "||AU - UD||_inf = " << err.array().abs().maxCoeff() );
    REQUIRE( err.array().abs().maxCoeff() == Approx(0.0) );
}

template <typename MatType>
void run_test_sets(const MatType& A, int k, int m)
{
    SECTION( "Largest Magnitude" )
    {
        run_test<MatType, LARGEST_MAGN>(A, k, m);
    }
    SECTION( "Largest Real Part" )
    {
        run_test<MatType, LARGEST_REAL>(A, k, m);
    }
    SECTION( "Largest Imaginary Part" )
    {
        run_test<MatType, LARGEST_IMAG>(A, k, m);
    }
    SECTION( "Smallest Magnitude" )
    {
        run_test<MatType, SMALLEST_MAGN>(A, k, m);
    }
    SECTION( "Smallest Real Part" )
    {
        run_test<MatType, SMALLEST_REAL>(A, k, m);
    }
    SECTION( "Smallest Imaginary Part" )
    {
        run_test<MatType, SMALLEST_IMAG>(A, k, m);
    }
}

TEST_CASE("Eigensolver of general real matrix [10x10]", "[eigs_gen]")
{
    std::srand(123);

    Matrix A = Eigen::MatrixXd::Random(10, 10);
    int k = 3;
    int m = 6;

    run_test_sets(A, k, m);
}

TEST_CASE("Eigensolver of general real matrix [100x100]", "[eigs_gen]")
{
    std::srand(123);

    Matrix A = Eigen::MatrixXd::Random(100, 100);
    int k = 10;
    int m = 20;

    run_test_sets(A, k, m);
}

TEST_CASE("Eigensolver of general real matrix [1000x1000]", "[eigs_gen]")
{
    std::srand(123);

    Matrix A = Eigen::MatrixXd::Random(1000, 1000);
    int k = 20;
    int m = 50;

    run_test_sets(A, k, m);
}

TEST_CASE("Eigensolver of sparse real matrix [10x10]", "[eigs_gen]")
{
    std::srand(123);

    const SpMatrix A = sprand(10, 0.5);
    int k = 3;
    int m = 6;

    run_test_sets(A, k, m);
}

TEST_CASE("Eigensolver of sparse real matrix [100x100]", "[eigs_gen]")
{
    std::srand(123);

    const SpMatrix A = sprand(100, 0.5);
    int k = 10;
    int m = 20;

    run_test_sets(A, k, m);
}

TEST_CASE("Eigensolver of sparse real matrix [1000x1000]", "[eigs_gen]")
{
    std::srand(123);

    const SpMatrix A = sprand(1000, 0.5);
    int k = 20;
    int m = 50;

    run_test_sets(A, k, m);
}
