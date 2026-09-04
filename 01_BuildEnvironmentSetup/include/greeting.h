// 01番の成果物である挨拶メッセージを生成するロジック。
//
// main()の中身をそのままテストするのは難しいため、出力する文字列の組み立てだけを
// ここに切り出し、GoogleTestから直接検証できるようにする。
#pragma once

#include <string>

// コンソールに表示する挨拶メッセージを返す。
std::string GetGreetingMessage();
