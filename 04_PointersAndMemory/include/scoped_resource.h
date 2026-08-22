#ifndef SCOPED_RESOURCE_H
#define SCOPED_RESOURCE_H

#include <string>

// RAII(Resource Acquisition Is Initialization)の実演用クラス。
// コンストラクタで「取得」、デストラクタで「解放」のログを出す。
// スコープを抜けるとき（正常終了・早期return・例外による巻き戻しのいずれでも）
// デストラクタは必ず呼ばれるため、後始末忘れが起きない。
class ScopedResource {
public:
    explicit ScopedResource(std::string name);
    ~ScopedResource();

    ScopedResource(const ScopedResource&) = delete;
    ScopedResource& operator=(const ScopedResource&) = delete;

private:
    std::string name_;
};

#endif  // SCOPED_RESOURCE_H
