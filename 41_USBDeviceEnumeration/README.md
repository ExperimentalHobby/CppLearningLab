# 41. USBデバイス列挙

## 目的
PCに接続されているUSBデバイスの一覧を取得し、USBデバイスプログラミングの
入り口を体験する。

## 学習ポイント
- USBデバイスの列挙API
- デバイスハンドルの取得と解放
- クロスプラットフォームUSBライブラリの基本的な使い方

## 採用ライブラリについて

README原案では`libusb`を推奨していたが、本課題では**Windows標準のSetupAPI**
(`setupapi.lib`、Windows SDK標準)を採用した。libusbで既存のUSBデバイス
(マウス等)にアクセスするには、デバイスドライバをHID/CDC等のクラスドライバから
WinUSBドライバへ差し替える必要があり、これは接続中の既存デバイスの動作を
壊すリスクがある。SetupAPIはOSが標準で提供するデバイス情報の読み取り専用APIで、
ドライバの変更を伴わずに安全にデバイス一覧を取得できる。

## 成果物イメージ
接続中のUSBデバイス一覧を、後述のUSBDeviceInfo(42番)で使う情報の
下準備として取得・表示するコマンドラインツール。

## ビルド方法・実行方法

他の課題と同様にCMakeを使う(詳細はルート [README.md](../README.md#cmakeとvisual-studioの使い分け) を参照)。

```sh
# 41_USBDeviceEnumeration ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/USBDeviceEnumeration
```

実行すると、接続中のUSBデバイスのインスタンスID(例: `USB\VID_046D&PID_C33C\197633433932`)
と説明文字列を一覧表示する。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `EnumerateUsbDevices()` | `SetupDiGetClassDevsW(nullptr, L"USB", ...)`でUSBバス配下のデバイスを列挙 | USBデバイスの列挙API |
| `SetupDiEnumDeviceInfo`のループ | デバイス情報セットのハンドルを1件ずつ列挙し、最後に`SetupDiDestroyDeviceInfoList`で解放 | デバイスハンドルの取得と解放 |
| `UsbDeviceEntry` | インスタンスID+説明を保持する最小限の構造体 | 列挙結果の受け渡し方 |

## 動作確認

この開発機に接続されている実際のUSBデバイス(マウス・キーボード・USBハブ・
プリンター・Webカメラ等、38件)に対してビルド済み`.exe`を実行し、日本語を
含む説明文字列も含めて文字化けせず一覧表示されることを確認した。

```
接続中のUSBデバイス: 38件

インスタンスID: USB\VID_0B05&PID_19AF&MI_02\6&DE2EAF5&0&0002
  説明         : USB 入力デバイス

インスタンスID: USB\VID_04A9&PID_1730&MI_01\9&3ACE4CB2&0&0001
  説明         : Canon MP540 series
...
```
