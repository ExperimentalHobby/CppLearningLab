# 46. 抜き差し検知

## 目的
USBデバイスの接続/切断をアプリケーションがリアルタイムに検知できるようにする。

## 学習ポイント
- Windowsの`WM_DEVICECHANGE`メッセージによるデバイス変更通知
- libusbのホットプラグAPI(`libusb_hotplug_register_callback`)によるクロス
  プラットフォームな検知
- 検知イベントに対する適切な後始末（切断時のハンドルクローズ等）

## 推奨ライブラリ/ツール
- Windows API（`RegisterDeviceNotification`）、または libusb hotplug API

## 成果物イメージ
特定のUSBデバイス（例: 特定のVID/PID）が接続/切断されるたびに、
コンソールまたはGUI（08. MultiThreadedGUIとの組み合わせも可）に通知を表示するツール。
