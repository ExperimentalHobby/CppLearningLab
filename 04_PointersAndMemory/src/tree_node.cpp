#include "tree_node.h"

TreeNode::TreeNode(std::string name) : name_(std::move(name)) {}

const std::string& TreeNode::Name() const {
    return name_;
}

void TreeNode::AddChild(const std::shared_ptr<TreeNode>& child) {
    child->parent_ = weak_from_this();
    children_.push_back(child);
}

const std::vector<std::shared_ptr<TreeNode>>& TreeNode::Children() const {
    return children_;
}

std::shared_ptr<TreeNode> TreeNode::Parent() const {
    // weak_ptr::lock()は、参照先がまだ生きていればshared_ptrを、
    // 既に破棄されていれば空のshared_ptrを返す。
    return parent_.lock();
}

long TreeNode::UseCount() const {
    // shared_from_this()で一時的なshared_ptrを作ると、その一時オブジェクト自体が
    // カウントを1つ増やしてしまい正確な値が取れない。weak_from_this()なら
    // 所有権を増やさずに現在の参照カウントを確認できる。
    return weak_from_this().use_count();
}
