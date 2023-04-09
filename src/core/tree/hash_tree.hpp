#ifndef CORE_TREE_HASH_TREE_HPP
#define CORE_TREE_HASH_TREE_HPP
#include "../../prereqs.hpp"
#include "../../core.hpp"

namespace openml {
namespace tree {

template<typename NodeType, typename DataType>
class HashTree {
private:
    std::size_t max_leaf_size_;
    std::size_t max_child_size_;

    std::shared_ptr<NodeType> root_;
    std::set<std::vector<DataType>> added_;
    std::vector<std::size_t> count_list_;
    std::vector<std::vector<DataType>> itemset_list_;

    inline const std::size_t hash(std::size_t num) const {
        return num % max_child_size_;
    }

protected:
    void insert(std::shared_ptr<NodeType>& node, 
        std::vector<DataType> itemset, 
        std::size_t count) {
        
        std::size_t key;
        // if current itemset is the last one, just insert it
        if (node->index == itemset.size()) {
            // if itemset is in bucket
            if (node->bucket.find(itemset) != node->bucket.end()) {
                node->bucket[itemset] += count;
            }
            else {
                node->bucket[itemset] = count;
            }
            return ;    
        }
        
        // non-leaf node 
        if (!node->is_leaf) {
            key = hash(itemset[node->index]);
            // if element k of current itemset is not in
            if (node->children.find(key) == node->children.end()) {
                node->children[key] = std::make_shared<NodeType>();
            }
            insert(node->children[key], itemset, count);
        }
        else {
            if (node->bucket.find(itemset) == node->bucket.end()) {
                node->bucket[itemset] = count;
            }
            else {
                node->bucket[itemset] += count;
            }

            if (node->bucket.size() > max_leaf_size_) {
                ++(node->index);
                // bucket is a map struct and key is vector
                for (auto& bucket : node->bucket) {
                    key = hash(bucket.first[node->index]);
                    if (node->children.find(key) == node->children.end()) {
                        node->children[key] = std::make_shared<NodeType>();
                    }
                    node->children[key]->index = std::min(node->index, bucket.first.size() - 1);
                    insert(node->children[key], bucket.first, bucket.second);
                }
                node->bucket = std::map<std::vector<DataType>, std::size_t>();
                node->is_leaf = false;

            }   
        }
    }

    void dfs(std::shared_ptr<NodeType> node, std::size_t support) {
        if (node->is_leaf) {
            for (auto& bucket : node->bucket) {
                if (bucket.second >= support) {
                    itemset_list_.emplace_back(bucket.first);
                    count_list_.emplace_back(bucket.second);
                }
            }
            return ;
        }
        else {
            for (auto& child : node->children) {
                dfs(child.second, support);
            }
        }
    }

public:
    HashTree(): max_leaf_size_(3), max_child_size_(3) {
        root_ = std::make_shared<NodeType>();
        root_->is_leaf = false;
    };

    HashTree(std::size_t max_leaf_size, 
        std::size_t max_child_size):
            max_leaf_size_(max_leaf_size), 
            max_child_size_(max_child_size) {
        root_ = std::make_shared<NodeType>();
        root_->is_leaf = false;
    };

    ~HashTree() {};

    void build_tree(const std::vector<std::vector<DataType>>& itemsets) {
        for (std::size_t i = 0; i < itemsets.size(); ++i) {
            insert(root_, itemsets[i], 0);
        }
    }

    void compute_frequency_itemsets(std::size_t support, 
        std::vector<std::size_t>& count_list,
        std::vector<std::vector<DataType>>& itemset_list) {
    
        dfs(root_, support);
        count_list = count_list_;
        itemset_list = itemset_list_;
    }

    
    void add_support(std::shared_ptr<NodeType> node, 
        std::vector<DataType> pick_itemset, 
        std::vector<DataType> rest_itemset, 
        std::size_t k) {
        
        if (node->is_leaf) {
            std::vector<DataType> superset = pick_itemset;
            superset.insert(superset.end(), rest_itemset.begin(), rest_itemset.end());

            for (auto& itemset : node->bucket) {
                if (added_.find(itemset.first) != added_.end()) {
                    continue;
                }
                std::vector<DataType> tmp;
                for (auto item : superset) {
                    if (item >= itemset.first[0] && item <= itemset.first.back().c) {
                        tmp.emplace_back(item);
                    }
                }

                for (auto item : itemset) {
                    if (std::find(tmp.begin(), tmp.end(), item) != tmp.end()) {
                        node->bucket[itemset]++;
                        added_.insert(itemset);
                    }
                }
            }
        }
        else {
            std::size_t num_pick_items = pick_itemset.size();
            std::size_t num_rest_items = rest_itemset.size();
            std::size_t min_num_rest_items = k - num_pick_items - 1;
            if (min_num_rest_items < 0) {
                return ;
            }

            std::size_t num_iters = num_rest_items - min_num_rest_items;

            for (std::size_t i = 0; i < num_iters; ++i) {
                std::vector<DataType> cur_pick = pick_itemset.emplace_back(rest_itemset[i]);
                std::vector<DataType> cur_rest = {rest_itemset.begin() + i, rest_itemset.end()};
                std::size_t key = hash(cur_pick[node->index]);
                if (node->children.find(key) != node->children.end()) {
                    add_support(node->children[key], cur_pick, cur_rest, k);
                }
            }
        }
    }
    

};

}
}
#endif /*CORE_TREE_HASH_TREE_HPP*/