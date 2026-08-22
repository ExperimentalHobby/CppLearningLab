#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <memory>
#include <string>
#include <vector>

// 親子関係を持つ木構造のノード。
//
// 子は「親が子を所有する」関係なので std::shared_ptr<TreeNode> で保持する。
// 一方、親への参照を単純に std::shared_ptr で持たせると
//   親 --shared_ptr--> 子 --shared_ptr--> 親
// という循環参照が生まれ、お互いを参照し合ったままuse_countが0にならず
// メモリリークする。これを避けるため、親への参照は所有権を持たない
// std::weak_ptr<TreeNode> で保持する。
class TreeNode : public std::enable_shared_from_this<TreeNode> {
public:
    explicit TreeNode(std::string name);

    const std::string& Name() const;

    // 子ノードを追加し、子の親ポインタ(weak_ptr)を自分に設定する。
    void AddChild(const std::shared_ptr<TreeNode>& child);

    const std::vector<std::shared_ptr<TreeNode>>& Children() const;

    // 親ノードを取得する。親が既に破棄されている、またはルートノードの場合はnullptr。
    std::shared_ptr<TreeNode> Parent() const;

    // このノードを指しているshared_ptrの数(参照カウント)。
    long UseCount() const;

private:
    std::string name_;
    std::weak_ptr<TreeNode> parent_;
    std::vector<std::shared_ptr<TreeNode>> children_;
};

#endif  // TREE_NODE_H
