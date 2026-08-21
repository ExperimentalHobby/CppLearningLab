#include <iostream>
#include <memory>
#include <stdexcept>

#include "linked_list.h"
#include "scoped_resource.h"
#include "tree_node.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

void DemoRawPointerAndReference() {
    std::cout << "=== 生ポインタ・参照・nullptr ===" << std::endl;

    int value = 42;
    int* rawPtr = &value;      // 生ポインタ: アドレスを持つが所有権の概念はない
    int& ref = value;          // 参照: valueの別名。null不可、再代入で別対象を指せない

    std::cout << "value = " << value << ", *rawPtr = " << *rawPtr << ", ref = " << ref
              << std::endl;

    *rawPtr = 100;
    std::cout << "*rawPtr = 100 とした後の value = " << value << std::endl;

    int* nullPtr = nullptr;
    if (nullPtr == nullptr) {
        std::cout << "nullPtrはnullptr（何も指していない）。デリファレンスするとUB。"
                  << std::endl;
    }

    // ダングリングポインタの例（実際にはデリファレンスしない）:
    // int* Dangling() {
    //     int local = 1;
    //     return &local;  // localは関数終了時に破棄されるため、返したポインタは
    //                      // 既に無効なメモリを指す(ダングリングポインタ)。
    // }
    std::cout << "ダングリングポインタ: ローカル変数のアドレスを関数外に返すと、"
                 "破棄済みメモリを指すポインタになる（実行はしない）。"
              << std::endl;
}

void DemoLinkedListComparison() {
    std::cout << "\n=== 生ポインタ版 vs スマートポインタ版の連結リスト ===" << std::endl;

    RawIntList rawList;
    rawList.PushFront(3);
    rawList.PushFront(2);
    rawList.PushFront(1);
    std::cout << "RawIntList:   " << rawList.ToString() << " (size=" << rawList.Size() << ")"
              << std::endl;
    std::cout << "  -> スコープを抜けるとデストラクタが手動でnew分をdeleteする。" << std::endl;

    SmartIntList smartList;
    smartList.PushFront(3);
    smartList.PushFront(2);
    smartList.PushFront(1);
    std::cout << "SmartIntList: " << smartList.ToString() << " (size=" << smartList.Size() << ")"
              << std::endl;
    std::cout << "  -> unique_ptrの連鎖なので、デストラクタを自分で書かなくても"
                 "自動的に全ノードが解放される。"
              << std::endl;
}

void DemoTreeWithSharedAndWeakPtr() {
    std::cout << "\n=== shared_ptr/weak_ptrによる木構造(循環参照の回避) ===" << std::endl;

    auto root = std::make_shared<TreeNode>("root");
    std::cout << "root作成直後の use_count = " << root->UseCount() << std::endl;

    auto child1 = std::make_shared<TreeNode>("child1");
    auto child2 = std::make_shared<TreeNode>("child2");
    root->AddChild(child1);
    root->AddChild(child2);

    std::cout << "root->AddChild後の root use_count = " << root->UseCount()
              << "（子はrootをweak_ptrで参照するため増えない）" << std::endl;
    std::cout << "child1 use_count = " << child1->UseCount()
              << "（root->children_ と、このスコープのchild1変数の2つが所有）" << std::endl;

    if (const auto parent = child1->Parent()) {
        std::cout << "child1の親: " << parent->Name() << std::endl;
    }

    std::cout << "rootの子: ";
    for (const auto& child : root->Children()) {
        std::cout << child->Name() << " ";
    }
    std::cout << std::endl;
}

void DemoRaii() {
    std::cout << "\n=== RAII: 通常スコープでの解放 ===" << std::endl;
    {
        ScopedResource resource("通常リソース");
        std::cout << "  リソースを使用中..." << std::endl;
    }  // ここでresourceのデストラクタが呼ばれ、「解放」ログが出る
    std::cout << "スコープを抜けた後" << std::endl;

    std::cout << "\n=== RAII: 例外発生時でも解放されることの確認 ===" << std::endl;
    try {
        ScopedResource resource("例外時リソース");
        std::cout << "  例外を送出します..." << std::endl;
        throw std::runtime_error("何らかのエラー");
        // ここには到達しないが、resourceのデストラクタは例外による巻き戻しの過程で
        // 必ず呼ばれる（後始末忘れが起きない）。
    } catch (const std::exception& e) {
        std::cout << "catchで捕捉: " << e.what() << std::endl;
    }
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    DemoRawPointerAndReference();
    DemoLinkedListComparison();
    DemoTreeWithSharedAndWeakPtr();
    DemoRaii();

    return 0;
}
