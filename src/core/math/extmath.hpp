#ifndef CORE_MATH_EXTMATH_HPP
#define CORE_MATH_EXTMATH_HPP
#include "../../prereqs.hpp"

namespace openml {
namespace math {

// template<typename DataType>
// double sigmoid(const DataType z) {
//     return DataType(1.0) / (DataType(1.0) + exp(-z));
// }


/**
 * compute the matrix sigmoid value
 *      s(z) = 1 / (1 + exp(-z))
 * @param x ndarray of shape [num_rows, num_cols]
 * @return sigmoid matrix 
*/
template<typename MatType, 
    typename DataType = typename MatType::value_type>
MatType sigmoid(const MatType& x) {
    return (static_cast<DataType>(1) / 
        (static_cast<DataType>(1) + (-x.array()).exp())).matrix();
};

/**
 * Axis or axes along which the variance is computed, 
 * if input data is a vector, return is a scalar, 
 * if input data is a matrtix, return is the covariamce 
 * matrix of ndarray.
 * 
 * The default is to compute the variance of the flattened array.
 * 
 * @param x input data of type vector or matrix 
 * @param axis int. default -1 The axis along which to calculate variance. 
 * @return scalar or 2darray
*/
template<typename MatType, 
    typename VecType, 
    typename DataType = typename MatType::value_type>
auto var(const MatType& x, int axis = -1) {

    // Var(X)=E[X^2]-(E[X])^2
    if (axis == 0) {
        
        std::size_t num_cols = x.cols();
        // compute means and element-wise square along to axis 1
        VecType col_mean(num_cols);
        col_mean = x.colwise().mean();
        VecType mean_x_squared(num_cols);
        mean_x_squared = x.array().square().colwise().mean().transpose().matrix();
        VecType col_var(num_cols);
        col_var = mean_x_squared - col_mean.array().square().matrix();
        return col_var;
    }
    else if (axis == 1) {
        std::size_t num_rows = x.rows();
        VecType row_mean(num_rows);
        row_mean = x.rowwise().mean();
        VecType mean_x_squared(num_rows);
        mean_x_squared = x.array().square().rowwise().mean().matrix();
        VecType row_var(num_rows);
        row_var = mean_x_squared - row_mean.array().square().matrix();
        return row_var;
    }
    else if (axis == -1) {
        std::size_t num_rows = x.rows(), num_cols = x.cols();
        MatType trans_x = x.transpose();
        VecType flatten_x(Eigen::Map<VecType>(trans_x.data(), num_rows * num_cols));
        VecType mean(1);
        mean = flatten_x.colwise().mean();
        VecType mean_x_squared(1);
        mean_x_squared = flatten_x.array().square().colwise().mean().matrix();
        VecType var(1);
        var = mean_x_squared - mean.array().square().matrix();
        return var;
    }
}

/**
 * Estimate a covariance matrix, given data.
 * Covariance indicates the level to which two variables vary together. 
 * If we examine N-dimensional samples, X = [x1, x2, .. x_n]_T , 
 * then the covariance matrix element C_ij is the covariance of x_i and x_j. 
 * The element C_ii is the variance of x_i.
 * 
 * @param x input data of type vector or matrix 
 * @return scalar or 2darray
*/
template<typename AnyType>
AnyType cov(const AnyType& x) {
    AnyType centered = x.rowwise() - x.colwise().mean();
    AnyType cov = (centered.adjoint() * centered) / static_cast<double>(x.rows() - 1);
    return cov;
};

/**
 * First array elements raised to powers from second param, element wise
 * Negative values raised to a non-integral value will return nan.
 * @param x input data of ndarray type
 * @param exponent double type 
 * 
 * @return The bases in x1 raised to the exponents
*/
template<typename AnyType>
AnyType power(const AnyType& x, double exponents) {
    return x.array().pow(exponents).matrix();
};

/**
 * Calculate centered matrix, where cerntering is done by substracting the mean 
 * over the colnum from each col of the matrix.
 * 
 * @param x Input matrix
 * @return centered matrix to write centered output
*/
template<typename MatType>
MatType center(const MatType& x) {
    return x.rowwise() - x.colwise().mean();
};

/**
 * Calculate the absolute value element-wise.
 * @param x ndarray of input data
 * @return An ndarray containing the absolute value of each element in x.
*/
template<typename AnyType>
AnyType abs(const AnyType& x) {
    return x.array().abs().matrix();
};

/**
 * Returns an element-wise indication of the sign of a number.
 * The sign function returns -1 if x < 0, 0 if x==0, 1 if x > 0.
 * @param x ndarray of input values
 * @return The sign of x.
*/
template<typename AnyType>
AnyType sign(const AnyType& x) {
    return x.array().sign().matrix();
};

/**
 * Sign correction to ensure deterministic output from SVD. 
 * Adjusts the columns of u and the rows of v such that the loadings in the
 * columns in u that are largest in absolute value are always positive.
 * 
 * @param U ndarray u and v are the output of `svd` with matching inner
 * dimensions so one can compute `Eigen::dot(u * s, v)`
 * 
 * @param u_based_decision Bool If True, use the columns of u as the basis for sign flipping. 
 * Otherwise, use the rows of v. The choice of which variable to base the 
 * decision on is generally algorithm dependent.
 * 
 * @return u_adjusted, v_adjusted : arrays with the same dimensions as the input.
*/
template<typename MatType, typename VecType, typename IndexType>
std::tuple<MatType, MatType> svd_flip(const MatType& U, 
    const MatType &Vt, 
    bool u_based_decision = true) {
    
    MatType U_, Vt_;

    if (u_based_decision) {
        // columns of u, rows of v
        MatType abs_U = abs<MatType>(U);
        IndexType max_abs_index = argmax<MatType, IndexType>(abs_U, 0);

        std::size_t num_elems = max_abs_index.rows();
        VecType max_abs_cols(num_elems);

        for(std::size_t j = 0; j < num_elems; j++) {
            std::size_t i = max_abs_index(j);
            max_abs_cols(j) = U(i, j);
        }
        VecType signs = sign(max_abs_cols);
        U_ = (U.array().rowwise() * signs.transpose().array()).matrix();        
        Vt_ = (Vt.array().colwise() * signs.array()).matrix();
    }
    else {
        // rows of v, columns of u
        MatType abs_Vt = abs<MatType>(Vt);
        IndexType max_abs_index = argmax<MatType, IndexType>(abs_Vt, 1);

        std::size_t num_elems = max_abs_index.rows();
        VecType max_abs_rows(num_elems);

        for(std::size_t i = 0; i < num_elems; i++) {
            std::size_t j = max_abs_index(i);
            max_abs_rows(i) = Vt(i, j);
        }

        VecType signs = sign(max_abs_rows);
        U_ = (U.array().rowwise() * signs.transpose().array()).matrix();        
        Vt_ = (Vt.array().colwise() * signs.array()).matrix();
    }
    return std::make_tuple(U_, Vt_);
};


}
}

#endif
