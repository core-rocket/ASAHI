# ASAHI
能代2019

## 注意事項

- 本プロジェクトではArduino Nanoの*互換品*を用いるため，以下の設定を行ってからプログラムをアップロードすること
	- ツール->ボード で"Arduino Nano"を選択
	- ツール->プロセッサ で"ATmega328P(Old Bootloader)"を選択

## ToDo

|Task|Progress|
|-|-|
|MPU6050: 加速度取得|Done|
|MPU6050: ジャイロ取得|Done|
|MPU6050: レンジ設定|Pending|
|MPU6050: DLPF使用|New|
|MPU6050: FIFO使用|New|
|TWE-Lite: 疎通確認|Done|
|TWE-Lite: 書式モードバイナリ形式での通信|Done|
|TWE-Lite: 文字列の送受信|Done|
|TWE-Lite: 数値の送受信|Done|
|TWE-Lite: 構造体の送受信|Done|
|TWE-Lite: 通信品質の取得|Done|
|GPS: ボードレート変更|Done|
|GPS: 出力パラメータ変更|Done|
|GPS: 出力データのパース|WIP|
|SD: ファイル書き込み|Done|
|SD: ファイル書き込み速度計測|Done|
|フライトピン: 動作確認|Done|
|BMP280: 気温取得|Done|
|BMP280: 気圧取得|Done|
|BMP280: 高度推定|Done|
|Servo: 動作確認|Done|
|ニクロム線: 動作確認(単独)|Done|
|ウォッチドッグ: 動作確認|New|
|バス部: 動作ステータス送信|New|
|バス部: 加速度・ジャイロデータ送信|Done|
|バス部: 加速度・ジャイロデータ保存|New|
|バス部: GPSデータ送信|WIP|
|バス部: GPSデータ保存|New|
|バス部: ミッション部からのデータ受信|New|
|バス部: BBM試験|New|
|バス部: ロングラン試験|New|
|ミッション部: データ送信(Toバス部)|Done|
|ミッション部: 高度移動平均|Done|
|ミッション部: 開傘タイムアウト|Done|
|ミッション部: リーフィングタイムアウト|Done|
|ミッション部: サーボ動作確認|Done|
|ミッション部: ニクロム線動作確認|WIP|
|ミッション部: BBM試験|Done|
|ミッション部: ロングラン試験|WIP|
|地上局: データ種別ごとにパース|WIP|
|地上局: データ保存|Done|
|地上局: 加速度・ジャイロのリアルタイムグラフ表示|Done|
|地上局: コマンド送信|New|
|地上局: 通信品質表示|New|
|地上局: パケットロスト数取得|New|
|地上局: GUI|New|
