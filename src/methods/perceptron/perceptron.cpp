#include "perceptron.hpp" 
using namespace perceptron;
using namespace math;

double Perceptron::sign(const arma::rowvec& x, 
    const arma::vec& w, 
    const double b) const {

    double y = arma::dot(x, w) + b;
    return (y >= 0.0) ? 1.0 : -1.0;
}

template<typename WeightInitializer>
const arma::vec Perceptron::train(const arma::mat& X, 
    const arma::vec& y, 
    WeightInitializer& weight_initializer) const {
    
    // get number of cols == n_features and number of samples
    std::size_t n_samples = X.n_rows;
    std::size_t n_features = X.n_cols;

    // define local weights and bias, and initialize them by
    // weight_initializer  
    arma::vec w;
    double b;

    // WeightInitializer weight_initializer;
    weight_initializer.Initialize(w, b, n_features);

    // std::cout << w << std::endl;
    // assigne X-shuffled and y_shuffled
    arma::mat X_shuffled = X;
    arma::vec y_shuffled = y;

    std::size_t iter = 0;

    while (iter < max_iter) {
        // shuffle dataset and associated label
        if (shuffle) {
            math::shuffle_data(X, y, X_shuffled, y_shuffled);
        }

        for (std::size_t i = 0; i < n_samples; i++) {
            arma::rowvec X_row = X_shuffled.row(i);
            double y_row = y_shuffled(i);
            double y_pred = sign(X_row, w, b);

            if ((y_row * y_pred) <= 0.0) {
                arma::vec X_trans = arma::conv_to<arma::vec>::from(X_row);
                w = w + alpha * X_trans * y_row;
                b = b + alpha * y_row;
            }
        }
        iter++;
    }
    w.resize(n_features + 1);
    w(n_features) = b;
    return w;
}

void Perceptron::fit(const arma::mat& X, 
    const arma::vec& y) {
    
    std::size_t n_features = X.n_cols;

    arma::vec retval(n_features + 1, arma::fill::zeros);

    if (initializer == "ones") {
        OnesInitializer ones_initializer;
        retval = train(X, y, ones_initializer);
    }
    else if (initializer == "rand") {
        RandInitializer rand_initializer;
        retval = train(X, y, rand_initializer);
    }

    

    weights = retval.subvec(0, n_features - 1);
    bias = retval(n_features);

    // std::cout << weights << std::endl;
    // std::cout << bias << std::endl;
}

const arma::mat Perceptron::predict(
    const arma::mat& X) const {

    arma::vec y_pred = X * weights + bias;

    for(auto& value:y_pred) {
        if (value >= 0.0) {
            value = 1.0;
        }
        else {
            value = 0;
        }
    }
    return y_pred;
}

const double Perceptron::score(
    const arma::vec &y_true, 
    const arma::vec &y_pred) const {
    
    double acc = 0.0;
    std::size_t n_samples = y_true.n_rows;

    for (int i = 0; i < n_samples; i++) {
        bool matched = (y_true(i) == y_pred(i));
        if (matched) {
            acc++;
        }
    }
    return acc / n_samples;
}
