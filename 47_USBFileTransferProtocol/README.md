# 47. 独自ファイル転送プロトコル

## 目的
38番で設計したバイナリプロトコルの考え方をUSB経由の通信に応用し、
USBデバイス越しにファイルを転送する仕組みを実装する集大成的な課題。

## 学習ポイント
- ファイルをチャンク分割して送信するプロトコル設計(38番の応用)
- 送達確認・再送・チェックサムによる転送の信頼性確保
- 44番のUSB CDC通信または43番のHID通信の上に構築

## 採用ライブラリ/ツール
- 38番のバイナリプロトコル(ヘッダー+ペイロード形式、`FrameParser`)+
  Windows標準のCNG(BCrypt) APIによるSHA-256チェックサム計算。

## 成果物イメージ
PCからArduino等のマイコンへ、あるいはマイコンからPCへ、小さなファイル
(テキストファイル等)をUSB経由で転送し、転送後にチェックサムで整合性を
確認するツール。

**本開発環境にはUSB経由で接続できる実機マイコンが無いため、実際のUSB
デバイス越しの転送は未確認**(詳細は「動作確認」を参照)。代わりに、
送信側でシリアライズしたバイト列をそのまま受信側の`FrameParser`へ給餌する
ことで、プロトコル自体の正しさ(チャンク分割→送信→受信→再構成→
チェックサム照合)を自己完結で確認するデモになっている。

## プロトコル仕様

38番と同じヘッダー(magic+version+command+reserved+length、12バイト)を使い、
`Command`をファイル転送用に定義している。

| コマンド | ペイロード形式 |
|---|---|
| `kFileStart` | `[2byte fileNameLen][fileName][8byte fileSize][2byte checksumLen][checksumHex]` |
| `kFileChunk` | `[4byte sequenceNumber][チャンクデータ]` |
| `kFileEnd` | (ペイロード無し、転送完了通知) |
| `kAck`/`kNack` | 4byte(シーケンス番号、送達確認・再送要求用。本実装では未使用の予約) |

## ビルド方法・実行方法

```sh
# 47_USBFileTransferProtocol ディレクトリで実行
cmake --preset x64-debug
cmake --build --preset x64-debug
./out/build/x64-debug/USBFileTransferProtocol <転送するファイルのパス>
```

指定したファイルを256バイトずつのチャンクに分割し、`kFileStart`+複数の
`kFileChunk`+`kFileEnd`としてシリアライズ→`FrameParser`で受信・再構成した
結果のチェックサムが元ファイルと一致することを表示する。

## 構成と学習ポイントとの対応

| 要素 | 内容 | 学習ポイント |
|---|---|---|
| `SplitIntoChunks()` | ファイル内容を固定サイズに分割 | チャンク分割設計 |
| `ComputeSha256Hex()` | Windows CNG(BCrypt)でSHA-256を計算 | チェックサムによる整合性確認 |
| `EncodeFileStart`/`DecodeFileStart` | ファイル名・サイズ・チェックサムの通知形式 | 転送前のメタデータ交換 |
| `EncodeFileChunk`/`DecodeFileChunk` | シーケンス番号付きチャンクの形式 | 順序が入れ替わりうる転送への対応 |
| `ReassembleChunks()` | シーケンス番号順に並べ替えてから結合 | 送達確認・再送を見据えた設計(順不同到着への耐性) |
| `binary_protocol.h`(38番と同じ枠組み) | ヘッダー+ペイロードのフレーミング、`FrameParser`によるストリーミング再構成 | 38番の応用 |

`kAck`/`kNack`は送達確認・再送のための仕組みとして定義しているが、
本実装(自己完結デモ)ではバイト列の欠落・破損が起きないため実際には
送出していない(実機のUSB通信で使うことを想定した予約)。

## 動作確認

- `test/file_transfer_test.cpp`(`ComputeSha256Hex`の既知テストベクトルとの
  一致、`SplitIntoChunks`、`EncodeFileStart`/`DecodeFileStart`、
  `EncodeFileChunk`/`DecodeFileChunk`、`ReassembleChunks`)と
  `test/binary_protocol_test.cpp`(フレーミング、複数メッセージの一括/分割
  受信、不正なマジックバイトでの例外、エンドツーエンドの往復確認)が
  合計20件全てパスすることを確認。
- 実機確認: 149バイトの小さいファイル(1チャンク)と2130バイトの
  ファイル(9チャンク、複数の`kFileChunk`メッセージにまたがる)の両方で
  `USBFileTransferProtocol.exe`を実行し、送信側チェックサムと受信側で
  再構成した内容のチェックサムが完全に一致することを確認した。

```
送信側: test_transfer_large.txt (2130バイト) を256バイトずつのチャンクに分割して送信します。
送信側チェックサム(SHA-256): 6fbb8bb2b6b5477b7aedf82681bd686bf887eaec2dc72c679b3435f9617a9748
受信側: 転送開始通知(ファイル名=test_transfer_large.txt, サイズ=2130バイト)
受信側: 転送終了通知=受信済み
受信側で再構成した内容のチェックサム: 6fbb8bb2b6b5477b7aedf82681bd686bf887eaec2dc72c679b3435f9617a9748
通知されたチェックサムとの一致: OK
元ファイルとの内容一致: OK
```

実際のUSBデバイス(マイコン等)越しの転送は、実機を接続できる環境で
43番(HID)または44番(USB CDC)の通信層と組み合わせて改めて確認する必要がある。
