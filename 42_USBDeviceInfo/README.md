# 42. デバイス情報取得

## 目的
USBデバイスのディスクリプタからVID/PID、製品名、シリアル番号などの
詳細情報を読み取れるようになる。

## 学習ポイント
- デバイスディスクリプタ、コンフィグレーションディスクリプタの構造
- VID(Vendor ID)/PID(Product ID)の意味と用途
- 文字列ディスクリプタ(製品名・メーカー名等)の取得

## 採用ライブラリについて

41番と同様の理由(README原案の`libusb`は既存デバイスのドライバ差し替えを
要求しリスクがある)により、Windows標準のSetupAPIを採用した。
デバイスディスクリプタを直接読むのではなく、SetupAPIがOS内部で保持している
デバイス情報(インスタンスID・レジストリプロパティ)からVID/PID等を取得する。

## 成果物イメージ
41番の列挙結果に対して、各デバイスのVID/PID・製品名・シリアル番号を
表示する詳細情報ツール。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う(詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照)。

```sh
# 42_USBDeviceInfo ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/USBDeviceInfo
```

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `ParseVidPid()` | インスタンスID文字列から`VID_XXXX`/`PID_XXXX`(16進数4桁)を抽出 | VID(Vendor ID)/PID(Product ID)の意味と用途 |
| `ExtractSerialNumber()` | インスタンスIDの末尾セグメント(シリアル番号、または複合デバイスのロケーション文字列)を抽出 | デバイスディスクリプタの構造(シリアル番号ディスクリプタ相当) |
| `GetManufacturer()`(`usb_device.cpp`) | `SetupDiGetDeviceRegistryPropertyW(SPDRP_MFG)`でメーカー名を取得 | 文字列ディスクリプタ(製品名・メーカー名等)の取得 |
| `GetDeviceDescription()`(`usb_device.cpp`) | `SPDRP_FRIENDLYNAME`/`SPDRP_DEVICEDESC`で製品名相当の説明を取得 | 文字列ディスクリプタの取得 |

`ParseVidPid`/`ExtractSerialNumber`はインスタンスID文字列に対する純粋な
文字列処理(SetupAPI呼び出しを含まない)であり、`test/`でGoogleTestによる
単体テストを行っている。

## 動作確認

- `test/usb_device_info_test.cpp`のテスト(単純なデバイス/複合デバイスからの
  VID・PID・シリアル番号抽出、大文字小文字の違い、不正な入力)が全てパスすることを確認。
- 実機確認: この開発機に接続されている実際のUSBデバイス(38件)に対して実行し、
  VID/PID・シリアル番号・メーカー名が正しく表示されることを確認した。

```
接続中のUSBデバイス: 38件

説明       : I-O DATA USB-PM560ER
  VID/PID  : VID_04BB PID_0943
  シリアル : 00064704
  メーカー : I-O DATA DEVICE INC

説明       : Canon MP540 series
  VID/PID  : VID_04A9 PID_1730
  シリアル : 9&3ACE4CB2&0&0001
  メーカー : Microsoft
...
```
