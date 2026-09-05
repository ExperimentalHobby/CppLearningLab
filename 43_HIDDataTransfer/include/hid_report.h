// HIDレポートのバイト列に対する純粋な処理(OS API呼び出しを含まない)。
// ReadFileで受信した生バイト列の解釈・表示用整形をここに集約し、単体テストできるようにする。
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hid {

// HIDのInput Reportは、レポートID(1バイト)+実データという形式が多い
// (デバイスがレポートIDを使わない場合は先頭バイトが実データの一部になるが、
// 本課題では「先頭バイトをレポートIDとして扱う」単純化したモデルで学習する)。
struct HidReport {
    uint8_t reportId = 0;
    std::vector<uint8_t> data;
};

// rawの先頭バイトをレポートIDとして分離する。rawが空の場合はreportId=0、
// dataも空のHidReportを返す。
HidReport ParseReport(const std::vector<uint8_t>& raw);

// バイト列を"01 02 0A"のような大文字16進数・スペース区切りの文字列に整形する
// (コンソール表示用)。
std::string FormatBytes(const std::vector<uint8_t>& bytes);

}  // namespace hid
