#include "tree_node.h"

#include <gtest/gtest.h>

TEST(TreeNodeTest, NewNodeHasNoParentAndNoChildren) {
    auto root = std::make_shared<TreeNode>("root");

    EXPECT_EQ(root->Name(), "root");
    EXPECT_EQ(root->Parent(), nullptr);
    EXPECT_TRUE(root->Children().empty());
}

TEST(TreeNodeTest, AddChildRegistersParentChildRelationship) {
    auto root = std::make_shared<TreeNode>("root");
    auto child = std::make_shared<TreeNode>("child");

    root->AddChild(child);

    ASSERT_EQ(root->Children().size(), 1u);
    EXPECT_EQ(root->Children()[0], child);
    EXPECT_EQ(child->Parent(), root);
}

// 親→子はshared_ptr、子→親はweak_ptrで持つため、rootを指すshared_ptrがなくなっても
// child経由でrootのメモリが生き残ったままにはならない(循環参照によるリークが起きない)。
TEST(TreeNodeTest, ParentIsReleasedWhenNoOwnerRemains) {
    auto child = std::make_shared<TreeNode>("child");
    {
        auto root = std::make_shared<TreeNode>("root");
        root->AddChild(child);
        EXPECT_NE(child->Parent(), nullptr);
    }  // root(shared_ptr)がスコープを抜けて破棄される。

    EXPECT_EQ(child->Parent(), nullptr);
}

// UseCount()はshared_from_this()経由の一時オブジェクトを作らずに参照カウントを返す。
TEST(TreeNodeTest, UseCountReflectsNumberOfOwners) {
    auto root = std::make_shared<TreeNode>("root");
    EXPECT_EQ(root->UseCount(), 1);

    { auto alias = root; EXPECT_EQ(root->UseCount(), 2); }

    EXPECT_EQ(root->UseCount(), 1);
}
