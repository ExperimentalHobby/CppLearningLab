# 46. 抜き差し検知

## 目的
USBデバイスの接続/切断をアプリケーションがリアルタイムに検知できるようにする。

## 学習ポイント
- Windowsの`WM_DEVICECHANGE`メッセージによるデバイス変更通知
- (参考)libusbのホットプラグAPI(`libusb_hotplug_register_callback`)による
  クロスプラットフォームな検知 → 本課題では不採用(41/42番と同じ理由)
- 検知イベントに対する適切な後始末(切断時のハンドルクローズ等)

## 採用ライブラリ/ツール
- Windows API(`RegisterDeviceNotification`/`WM_DEVICECHANGE`)。11-18番と
  同じWin32 GUIアプリの構成。

## 成果物イメージ
USBデバイスが接続/切断されるたびに、GUI上のリストへ時刻・種別(接続/切断)・
VID/PID・デバイスパスを追記して表示するツール。

## ビルド方法・実行方法

```sh
# 46_USBHotplugDetection ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/USBHotplugDetection
```

起動するとウィンドウが表示され、以降USBデバイスを抜き差しするたびにリストへ
イベントが追記される。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `RegisterForDeviceNotifications()` | `RegisterDeviceNotificationW`+`GUID_DEVINTERFACE_USB_DEVICE`で通知を登録 | `WM_DEVICECHANGE`によるデバイス変更通知 |
| `WndProc`の`WM_DEVICECHANGE`処理 | `DBT_DEVICEARRIVAL`(接続)/`DBT_DEVICEREMOVECOMPLETE`(切断)を判別 | 検知イベントの種別判定 |
| `WM_DESTROY`での`UnregisterDeviceNotification` | ウィンドウ破棄時に通知登録を解除する後始末 | 検知イベントに対する適切な後始末 |
| `usb_device_info.h`(42番と共通)の`ParseVidPid` | 通知されたデバイスパスからVID/PIDを抽出 | 42番の知識の再利用 |

`GUID_DEVINTERFACE_USB_DEVICE`の実体をこの翻訳単位でリンクするため、
`usbiodef.h`より前に`initguid.h`をインクルードしている(Windows SDKの定番作法)。

## 動作確認

- `ParseVidPid`/`ExtractSerialNumber`のテスト(42番と共通、8件)が全てパスすることを確認。
- 実機確認: この開発機はVM上で動作しており、Claudeからは物理的なUSBデバイスの
  抜き差し操作ができないため、**実際の接続/切断イベントの検知は未確認**。
  ビルド済み`.exe`を実際に起動し、ウィンドウとリストビュー(時刻/イベント/
  VID/PID/デバイスパスの4列)が正しく表示され、`RegisterDeviceNotificationW`
  の登録処理を含む`WM_CREATE`がクラッシュ無く完了することは
  `EnumWindows`+スクリーンショットで確認した。

実際の抜き差し検知は、USBメモリ等を物理的に操作できる環境で改めて確認する
必要がある。
