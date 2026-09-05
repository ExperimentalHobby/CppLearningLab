# 43. HIDデータ送受信

## 目的
HID(Human Interface Device)クラスのUSBデバイス(マウス、キーボード、
汎用HID機器)とのレポート送受信を実装する。

## 学習ポイント
- HIDレポートの構造(Input/Output/Feature Report)
- Windows標準HID API(`hid.lib`)を使ったデバイスのオープンとレポート送受信
- レポートディスクリプタの読み方(任意、余裕があれば → 本課題ではスコープ外)

## 採用ライブラリについて

README原案では`hidapi`を推奨していたが、hidapiは内部的にはWindows標準の
HID API(`hid.lib`)とSetupAPI(`setupapi.lib`、いずれもWindows SDK標準)で
実装されている。本課題ではそれらを直接使い、外部ライブラリを追加していない。

## 成果物イメージ
市販のHID対応デバイス(ゲームパッド等)や、Arduino等で自作したHID
デバイスからの入力データを読み取り、コンソールに表示するプログラム。

## ビルド方法・実行方法

```sh
# 43_HIDDataTransfer ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/HIDDataTransfer
```

接続中のHIDデバイス一覧(VID/PID・製品名・メーカー名)が表示されるので、
番号を入力するとそのデバイスからInput Reportを5回読み取って表示する
(何も入力せずEnterのみで終了)。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `EnumerateHidDevices()` | `HidD_GetHidGuid`+`SetupDiGetClassDevs`でHIDインターフェースを列挙し、`HidD_GetAttributes`/`HidD_GetProductString`/`HidD_GetManufacturerString`で情報取得 | デバイスのオープンと情報取得 |
| `ReadReports()` | `HidD_GetPreparsedData`+`HidP_GetCaps`でレポート長を取得し、`ReadFile`でInput Reportを読み取る | HIDレポートの送受信 |
| `ParseReport()`/`FormatBytes()`(`hid_report.h`) | レポートID+データへの分解、16進数文字列への整形 | HIDレポートの構造(Input Report) |

`ParseReport`/`FormatBytes`はバイト列に対する純粋関数(HID API呼び出しを
含まない)であり、`test/`でGoogleTestによる単体テストを行っている。

## 動作確認

- `test/hid_report_test.cpp`のテスト(レポートID/データの分離、空データ、
  16進数整形)が全てパスすることを確認。
- 実機確認: この開発機に接続されている実際のHIDデバイス(マウス・キーボード
  等、33件)に対して`HIDDataTransfer.exe`を実行し、VID/PID・製品名・
  メーカー名(例: `G512 RGB MECHANICAL GAMING KEYBOARD (Logitech)`)が
  正しく列挙されることを確認した。
- レポートの実読み取り(`ReadFile`)は、デバイスのオープン自体には成功し
  `ReadFile`がデータ到着待ちで正しくブロックすることを確認したが、実際に
  レポートが届くには物理的なマウス操作等が必要であり、この開発環境
  (自動化ツールからの操作)では確認できなかった。OSの合成入力API
  (`mouse_event`)で代用を試みたが、これは論理的な入力イベントを生成する
  ものであり、物理デバイスのHIDレポートストリームには反映されないため、
  代替検証にはならなかった。手動でマウス等を操作しながら実行すれば、
  実際のレポートが読み取れることが期待される。
