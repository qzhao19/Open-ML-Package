#ifndef CORE_TREE_KD_TREE_HPP
#define CORE_TREE_KD_TREE_HPP
#include "../../prereqs.hpp"
#include "../../core.hpp"

namespace openml {
namespace tree {

template<typename DataType>
class KDTree {
private:
    struct KDTreeNode {
        int left;
        int right;
        std::shared_ptr<std::vector<std::size_t>> indices;
        std::shared_ptr<std::vector<std::vector<DataType>>> data;
        std::shared_ptr<std::vector<std::vector<DataType>>> left_hyper_rect;
        std::shared_ptr<std::vector<std::vector<DataType>>> right_hyper_rect;

        KDTreeNode(): left(-1), 
            right(-1), 
            indices(nullptr), 
            data(nullptr), 
            left_hyper_rect(nullptr), 
            right_hyper_rect(nullptr) {};

        KDTreeNode(int left_, 
            int right_, 
            std::shared_ptr<std::vector<std::size_t>> indices_, 
            std::shared_ptr<std::vector<std::vector<DataType>>> data_, 
            std::shared_ptr<std::vector<std::vector<DataType>>> left_hyper_rect_, 
            std::shared_ptr<std::vector<std::vector<DataType>>> right_hyper_rect_): 
                left(left_), 
                right(right_), 
                data(data_), 
                left_hyper_rect(left_hyper_rect_), 
                right_hyper_rect(right_hyper_rect_) {};
        
        ~KDTreeNode() {};
    };

    struct StackDataNode {
        bool is_left;
        std::size_t depth;
        std::size_t parent;
        std::vector<std::size_t> indices;
        std::vector<std::vector<DataType>> data;
    };


    std::size_t leaf_size_;
    std::vector<KDTreeNode> tree_;
    const std::vector<std::vector<DataType>> data_;
    

protected:

    std::size_t find_partition_axis(const std::vector<std::vector<DataType>>& data) {
        std::size_t num_samples = data.size();
        std::size_t num_features = data[0].size();
        std::vector<DataType> range_bounds(num_features);
        std::vector<DataType> lower_bounds(num_features, ConstType<DataType>::infinity());
        std::vector<DataType> upper_bounds(num_features, -ConstType<DataType>::infinity());

        for (std::size_t i = 0; i < num_samples; ++i) {
            for (std::size_t j = 0; j < num_features; ++j) {
                lower_bounds[j] = std::min(lower_bounds[j], data[i][j]);
                upper_bounds[j] = std::max(upper_bounds[j], data[i][j]);
            }
        }

        std::size_t partition_axis = 0;
        for (std::size_t i = 0; i < num_features; ++i) {
            range_bounds[i] = std::abs(upper_bounds[i] - lower_bounds[i]);
            if (range_bounds[i] > range_bounds[partition_axis]) {
                partition_axis = i;
            }
        }

        return partition_axis;
    };


    const std::vector<std::size_t> argsort_data(const std::vector<DataType>& data) {
        
        std::vector<std::size_t> indices;
        std::vector<std::pair<DataType, std::size_t>> combine;
        for (int i = 0; i < data.size(); ++i) {
            combine.push_back(std::make_pair(data[i], i));
        }
        std::sort(combine.begin(), combine.end(), [](const auto &left, const auto &right) {
            return left.first < right.first;
        });

        for (int i = 0; i < itemsets_list.size(); ++i) {
            indices[i] = combine[i].second;
        }

        return indices;
    }


    void build_tree() {

        std::vector<std::vector<DataType>> data = data_;
        std::size_t num_samples = data.size();
        std::size_t num_features = data[0].size();
        
        // find bounding hyper-rectangle
        std::vector<DataType> lower_bounds(num_features, ConstType<DataType>::infinity());
        std::vector<DataType> upper_bounds(num_features, -ConstType<DataType>::infinity());

        for (std::size_t i = 0; i < num_samples; ++i) {
            for (std::size_t j = 0; j < num_features; ++j) {
                lower_bounds[j] = std::min(lower_bounds[j], data[i][j]);
                upper_bounds[j] = std::max(upper_bounds[j], data[i][j]);
            }
        }

        std::vector<std::vector<DataType>> hyper_rect(2, std::vector<DataType>(num_features));
        hyper_rect[0].emplace_back(lower_bounds);
        hyper_rect[1].emplace_back(upper_bounds);

        // create root of kd-tree
        std::size_t partition_axis = find_partition_axis(data);
        std::vector<DataType> partition_data;

        for (const auto& row : data) {
            partition_data.emplace_back(row[partition_axis]);
        }
        std::vector<std::size_t> indices = argsort_data(partition_data);

        for (std::size_t i = 0; i < indices.size(); ++i) {
            data[i] = data[indices[i]];
        }

        std::size_t mid_idx = num_samples / 2;
        DataType partition_val = data[mid_idx][partition_axis];

        std::vector<std::vector<DataType>> left_hyper_rect, right_hyper_rect;
        left_hyper_rect = hyper_rect;
        right_hyper_rect = hyper_rect;
        left_hyper_rect[1][0] = partition_val;
        right_hyper_rect[0][0] = partition_val;

        KDTreeNode node;
        node.left_hyper_rect = left_hyper_rect;
        node.right_hyper_rect = right_hyper_rect;

        tree_.emplace_back(node);

        StackDataNode stack_data1, stack_data2;

        stack_data1.is_left = true;
        stack_data1.depth = 1;
        stack_data1.parent = 0;
        stack_data1.data = data;
        stack_data1.indices = indices;

        stack_data2.is_left = false;
        stack_data2.depth = 1;
        stack_data2.parent = 0;
        stack_data2.data = data;
        stack_data2.indices = indices;

        std::stack<StackDataNode> stack;
        stack.push(stack_data1);
        stack.push(stack_data2);

        // recursively split data in halves using hyper-rectangles:
        while (!stack.empty()) {
            
            // pop data off stack
            StackDataNode tmp_stack_data = stack.top();
            stack.pop();

            num_samples = tmp_stack_data.data.size();
            std::size_t node_ptr = tree_.size();

            // update parent node
            KDTreeNode tmp_node;
            tmp_node = tree_[tmp_stack_data.parent];
            
            if (tmp_stack_data.is_left) {
                KDTreeNode node1;
                node1.left = node_ptr;
                node1.right = tmp_node.right;
                node1.indices = tmp_node.indices;
                node1.data = tmp_node.data;
                node1.left_hyper_rect = tmp_node.left_hyper_rect;
                node1.right_hyper_rect = tmp_node.right_hyper_rect;
                tree_[tmp_stack_data.parent] = node1;
            }
            else {
                KDTreeNode node2;
                node2.left = tmp_node.left;
                node2.right = node_ptr;
                node2.indices = tmp_node.indices;
                node2.data = tmp_node.data;
                node2.left_hyper_rect = tmp_node.left_hyper_rect;
                node2.right_hyper_rect = tmp_node.right_hyper_rect;
                tree_[tmp_stack_data.parent] = node2;
            }

            // insert node in kd-tree
            // leaf node?
            if (num_samples <= leaf_size_) {
                KDTreeNode leaf;
                leaf.data = tmp_node.data;
                leaf.indices = tmp_node.indices;
                leaf.left_hyper_rect = nullptr;
                leaf.right_hyper_rect = nullptr;
                leaf.left = 0;
                leaf.right = 0;
                tree_.emplace_back(leaf);
            }
            // not a leaf, split the data in two 
            else {
                partition_axis = find_partition_axis(tmp_node.data);
                partition_data.clear();
                for (const auto& row : tmp_node.data) {
                    partition_data.emplace_back(row[partition_axis]);
                }
                // indices = argsort_data(partition_data);


                
                node_ptr = tree_.size();

                StackDataNode stack_data3, stack_data4;

                stack_data3.is_left = true;
                stack_data3.depth = tmp_node.depth + 1;
                stack_data3.parent = node_ptr;
                stack_data3.data = tmp_node.data;
                stack_data3.indices = tmp_node.indices;

                stack_data4.is_left = false;
                stack_data4.depth = tmp_node.depth + 1;
                stack_data4.parent = node_ptr;
                stack_data4.data = tmp_node.data;
                stack_data4.indices = tmp_node.indices;


            }


        }




    }



public:
    KDTree(const std::vector<std::vector<DataType>>& data, 
        std::size_t leaf_size): data_(data), leaf_size_(leaf_size) {};

    KDTree(const std::vector<std::vector<DataType>>& data): 
        data_(data), leaf_size_(10) {};

    ~KDTree() {
        data_.clear();
    };


    void test() {

        std::size_t axis = find_partition_axis(data_);

        std::cout << axis << std::endl;

        build_tree();

    };



};


}
}
#endif /*CORE_TREE_KD_TREE_HPP*/