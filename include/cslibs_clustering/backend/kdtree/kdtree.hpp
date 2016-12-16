#pragma once

#include <cslibs_clustering/backend/options.hpp>
#include <cslibs_clustering/backend/kdtree/kdtree_options.hpp>
#include <cslibs_clustering/interface/data/data.hpp>
#include <limits>

namespace cslibs_clustering
{
namespace backend
{
namespace kdtree
{

template<typename data_interface_t_, typename index_interface_t_, typename... options_ts_>
class KDTree
{
public:
    using data_if = data_interface_t_;
    using data_t = typename data_if::type;

    using index_if = index_interface_t_;
    using index_t = typename index_if::type;

    using split_value_t = option::get_option<option::split_value_type_opt, options_ts_...>;
    static constexpr auto on_duplicate_index_strategy = option::get_option<option::merge_strategy_opt, options_ts_...>::value;

private:
    struct Node
    {
        inline constexpr bool is_leaf() const { return (left == nullptr) & (right == nullptr); }
        inline constexpr bool branch_left(const index_t& index) const { return index[split_dimension] < split_value; }

        inline Node* split(Node* left, Node* right, const index_t& index)
        {
            {
                split_value_t max_delta = 0;
                for (std::size_t i = 0; i < index_if::dimensions; ++i)
                {
                    auto delta = std::abs(index_if::access(i, this->index) - index_if::access(i, index));
                    if (delta > max_delta)
                    {
                        max_delta = delta;
                        split_dimension = i;
                    }
                }

                split_value = (index_if::access(split_dimension, this->index) + index_if::access(split_dimension, index)) / split_value_t(2);
            }

            this->left = left;
            this->right = right;

            if (branch_left(this->index))
            {
                std::swap(left->data, this->data);
                left->index = this->index;

                right->index = index;
                return right;
            }
            else
            {
                std::swap(right->data, this->data);
                right->index = this->index;

                left->index = index;
                return left;
            }
        }

        Node* left = nullptr;
        Node* right = nullptr;
        split_value_t split_value = {};
        std::size_t split_dimension = {};

        index_t index;
        data_t data;
    };

public:
    ~KDTree()
    {
        clear();
    }

    template<typename... Args>
    inline data_t& insert(const index_t& index, Args&&... args)
    {
        if (root_ == nullptr)
        {
            root_ = new Node();
            root_->index = index;
            data_if::template merge<option::MergeStrategy::REPLACE>(root_->data, std::forward<Args>(args)...);
            return root_->data;
        }
        else
        {
            Node* current = root_;
            while (!current->is_leaf())
            {
                if (current->branch_left(index))
                    current = current->left;
                else
                    current = current->right;
            }

            if (current->index != index)
                current = current->split(new Node(), new Node(), index);

            data_if::template merge<on_duplicate_index_strategy>(current->data, std::forward<Args>(args)...);

            return current->data;
        }
    }

    inline data_t* get(const index_t& index)
    {
        if (root_ == nullptr)
            return nullptr;

        Node* current = root_;
        while (!current->is_leaf())
        {
            if (current->branch_left(index))
                current = current->left;
            else
                current = current->right;
        }
        if (current->index != index)
            return nullptr;

        return &(current->data);
    }

    inline const data_t* get(const index_t& index) const
    {
        if (root_ == nullptr)
            return nullptr;

        const Node* current = root_;
        while (!current->is_leaf())
        {
            if (current->branch_left(index))
                current = current->left;
            else
                current = current->right;
        }
        if (current->index != index)
            return nullptr;

        return &(current->data);
    }

    template<typename Fn>
    inline void traverse(const Fn& function)
    {
        if (root_)
            traverse(function, root_);
    }

    template<typename Fn>
    inline void traverse(const Fn& function) const
    {
        if (root_)
            traverse(function, root_);
    }

    inline void clear()
    {
        if (root_)
        {
            clear(root_);
            delete root_;
        }
    }

private:
    template<typename Fn>
    inline void traverse(const Fn& function, Node* node)
    {
        if (node->is_leaf())
            function(node->index, node->data);
        else
        {
            traverse(function, node->left);
            traverse(function, node->right);
        }
    }

    template<typename Fn>
    inline void traverse(const Fn& function, const Node* node) const
    {
        if (node->is_leaf())
            function(node->index, node->data);
        else
        {
            traverse(function, node->left);
            traverse(function, node->right);
        }
    }

    inline void clear(Node* node)
    {
        if (!node->is_leaf())
        {
            clear(node->left);
            delete node->left;
            clear(node->right);
            delete node->right;
        }
    }

private:
    Node* root_ = nullptr;
};

}
}
}