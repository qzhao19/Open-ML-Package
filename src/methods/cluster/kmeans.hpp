#ifndef METHODS_CLUSTER_KMEANS_HPP
#define METHODS_CLUSTER_KMEANS_HPP
#include "../../prereqs.hpp"
#include "../../core.hpp"
using namespace openml;

namespace openml {
namespace cluster {

/**
 * K-Means clustering.
*/
template<typename DataType>
class KMeans {
private:
    // define matrix and vector Eigen type
    using MatType = Eigen::Matrix<DataType, Eigen::Dynamic, Eigen::Dynamic>;
    using VecType = Eigen::Matrix<DataType, Eigen::Dynamic, 1>;
    using IdxType = Eigen::Vector<Eigen::Index, Eigen::Dynamic>;
    
    std::string init_;
    std::size_t num_init_;
    std::size_t num_clusters_;
    std::size_t max_iter_;
    double tol_;
    
    MatType centroids_;

protected:
    void init_centroid(const MatType& X, 
        const VecType& x_squared_norms) {
        
        std::size_t num_samples = X.rows(), num_features = X.cols(); 
        MatType centroids(num_clusters_, num_features);

        if (init_ == "random") {
            IdxType index = math::permutation<IdxType>(num_samples);
            IdxType selected_index = index.topRows(num_clusters_);
            centroids = X(selected_index, Eigen::all);
        }   
        centroids_ = centroids;
    }
    
    /**
     * k-means lloyd
    */
    void kmeans_lloyd(const MatType& X) {

        // bool converged = false;
        std::size_t num_samples = X.rows(), num_features = X.cols(); 
        // MatType new_centroid(num_clusters_, num_features);

        for (std::size_t iter = 0; iter < max_iter_; ++iter) {
            // define the cluster
            std::map<std::size_t, std::vector<std::size_t>> clusters;
            for (std::size_t i = 0; i < num_samples; ++i) {
                std::size_t min_dist_index = 0;
                double min_dist = ConstType<double>::infinity();
                // compute the min distance between sample and centroid
                for (std::size_t j = 0; j < num_clusters_; ++j) {
                    MatType tmp;
                    tmp = X.row(i).array() - centroids_.row(j).array();
                    double dist = static_cast<double>(tmp.norm());
                    if (dist < min_dist) {
                        min_dist = dist;
                        min_dist_index = j;
                    }
                }
                clusters[min_dist_index].push_back(i);
            }
            // move the centers 
            double eps = 0.0; 
            for (auto& c : clusters) {
                MatType cluster = X(c.second, Eigen::all);
                MatType mean_c;
                mean_c = math::mean<MatType, VecType>(cluster, 0);

                MatType diff;
                diff = mean_c.array() - centroids_.row(c.first).array();
                
                eps += static_cast<double>(diff.norm());
                centroids_.row(c.first) = mean_c.transpose();
            }
            // convergen condition
            if (eps < tol_) {
                break;
            }
        }
    }

    /**
     * predict label
    */
    const VecType predict_label(const MatType& X) const{
        std::size_t num_samples = X.rows();
        VecType y_pred(num_samples);
        for (std::size_t i = 0; i < num_samples; ++i) {
            std::size_t min_dist_index = 0;
            double min_dist = ConstType<double>::infinity();

            for (std::size_t j = 0; j < num_clusters_; ++j) {
                MatType diff;
                diff = X.row(i).array() - centroids_.row(j).array();
                double dist = static_cast<double>(diff.norm());
                if (dist < min_dist) {
                    min_dist = dist;
                    min_dist_index = j;
                }
            }
            y_pred(i) = min_dist_index;
        }
        return y_pred;
    }




public:
    KMeans(): init_("random"), 
        num_init_(10),
        num_clusters_(3), 
        max_iter_(300), 
        tol_(1e-4) {};

    KMeans(std::string init,
        std::size_t num_init,
        std::size_t num_clusters,
        std::size_t max_iter, 
        double tol): init_(init), 
            num_init_(num_init),
            num_clusters_(num_clusters), 
            max_iter_(max_iter), 
            tol_(tol) {};


    void test_func(const MatType& X) {
        init_centroid(X, VecType());

        // MatType new_centroids;
        kmeans_lloyd(X);

        VecType y_pred = predict_label(X);

        std::cout << "y_pred" << std::endl;
        std::cout << y_pred << std::endl;
    }





};

} // cluster_model
} // openml

#endif /*METHODS_CLUSTER_KMEANS_HPP*/
