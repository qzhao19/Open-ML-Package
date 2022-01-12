#ifndef CORE_UTILS_HPP
#define CORE_UTILS_HPP
#include "../prereqs.hpp"

namespace openml {
namespace utils {

/**
 *find the maximum value in a std::map and return the corresponding std::pair
*/
template <class Container>
auto max_element(Container const &x)
    -> typename Container::value_type {
    
    using value_t = typename Container::value_type;
    const auto compare = [](value_t const &p1, value_t const &p2)
    {
        return p1.second < p2.second;
    };
    return *std::max_element(x.begin(), x.end(), compare);
};


/**
 * Stack arrays in sequence horizontally (column wise).
 * 
 * @param mat1_mat2 ndarray of shape (n_rows, n_cols) The arrays must have the same shape along all 
 * @return stacke dndarray
*/
template<typename MatType>
MatType hstack(const MatType& mat1, const MatType& mat2) {
    assert(mat1.rows() == mat2.rows() && "hstack with mismatching number of rows");
    std::size_t n_rows = mat1.rows(), n_cols1 = mat1.cols(), n_cols2 = mat2.cols();
    MatType retval(n_rows, (n_cols1 + n_cols2));
    retval << mat1, mat2;
    return retval;
};

/**
 * Stack arrays in sequence vertically (row wise). This is equivalent to concatenation 
 * along the first axis after 1-D arrays of shape (N,) have been reshaped to (1,N).
 * 
 * @param mat1_mat2 ndarray of shape (n_rows, n_cols) The arrays must have the same shape along all 
 * @return stacke dndarray
*/
template<typename MatType>
MatType vstack(const MatType& mat1, const MatType& mat2) {
    assert(mat1.cols() == mat2.cols() && "vstack with mismatching number of columns");
    std::size_t n_cols = mat1.cols(), n_rows1 = mat1.rows(), n_rows2 = mat2.rows();
    MatType retval((n_rows1 + n_rows2), n_cols);
    retval << mat1, 
              mat2;
    return retval;
};

/**
 * flatten a 2D vector of vector to a one dimension vector
*/
template<typename DataType, typename = typename DataType::value_type>
DataType flatten(const std::vector<DataType>& v) {
    return std::accumulate(v.begin(), v.end(), DataType{}, [](auto& dest, auto& src) {
        dest.insert(dest.end(), src.begin(), src.end());
        return dest;
    });
};

/**
 * Repeat elements of an matrix.
 * 
 * @param mat Input array.
 * @param repeats int. The number of repetitions for each element. 
 * @param axis int. The axis along which to repeat values. 
 * 
 * @return  Output array which has the same shape as a, except along the given axis.
*/
template<typename MatType>
MatType repeat(const MatType& mat, 
    int repeats, int axis) {
    
    MatType retval;
    if (axis == 0) {
        retval = mat.colwise().replicate(repeats);
    }
    else if (axis == 1) {
        retval = mat.rowwise().replicate(repeats);
    }
    return retval;
};



}
}
#endif /**/