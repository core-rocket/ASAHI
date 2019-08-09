# MPU6050

## リンク
- [回路図](https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/IMU/MPU-6050_Breakout%20V11.pdf)
- [仕様書(3.1)](https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Components/General%20IC/PS-MPU-6000A.pdf)
- [仕様書(3.4)](https://www.invensense.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [レジスタマップと説明(4.2)](https://www.invensense.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf)

MPU: Motion Processing Unit

- I2Cでアクセス(MPU-6000はSPIもいける)
- 3軸ジャイロ，3軸加速度が取れる
- 取れるデータは16bit．ADC(analog-to-digital converter)によりデジタルデータになっている
- ジャイロ，角速度共にデータ範囲をユーザが設定できる
	- ジャイロ:	±250, ±500, ±1000, ±2000 °/sec (dps)
	- 加速度:	±2, ±4, ±8, ±16 g
- 1024バイトのFIFO
- モーションプロセッシングの計算をオフロードすることができる，頻繁にポーリングする必要も無い
- プログラマブルなローパスフィルタ
- 加速度センサによる割り込み
	- プログラマブル
	- 自由落下
	- 高G
	- 静止
- ジャイロと加速度のサンプリングレートはユーザー定義可能
- DMP(Digital Motion Processor)を搭載
	- こいつが色々オフロードしてくれる
	- 計算結果はDMPのレジスタ，もしくはFIFOから読める
	- DMPを200Hzで動かしてマイコンは5Hzで見に行く，とかできる

## 処理にかかる時間

- I2Cは通常100kHzだが，設定で400kHzにもできる

|概要|処理時間(ms)|400kHz版|
|-|-|-|
|14byte(2(3+1+3))連続で読む|1.72〜1.73|0.55|

## レジスタ
- センサデータレジスタ
	- ジャイロ,加速度,補助(?),温度の最新のデータ
	- リードオンリ
- FIFOコンフィグレーション
	- どのデータをFIFOに書くか設定
- 割り込みコンフィグレーション
- 割り込みステータス

## サンプリングレート
- ジャイロ出力レート:
	- 8kHz(DLPF disabled)
	- 1kHZ(DLPF enabled)
- 加速度出力レート: 1kHz(const)
- SMPLRT_DIV: unsigned 8bit

サンプリングレート(レジスタやFIFOの更新頻度) = ジャイロ出力レート / (1 + SMPLRT_DIV)

- DLPF有効時のサンプリングレート: 1000〜4

### DLPF
- Digital Low Pass Filter
- CONFIGレジスタの下位3bitのDLPF_CFGで設定

|DLPF_CFG|加速度レート|遅延(ms)|ジャイロレート|遅延(ms)|
|-:|-:|-:|-:|-:|
|0(無効)|			260|		0|			256|	0.98|
|	1	|			184|	2.0|			188|	1.9|
|	2	|			94|		3.0|			98|		2.8|
|	3	|			44|		4.9|			42|		4.8|
|	4	|			21|		8.5|			20|		8.3|
|	5	|			10|		13.8|			10|		13.4|
|	6	|			5|		19.0|			5|		18.6|
|7(無効)|	reserved|	reserved|	reserved|	reserved|

## プログラマブル割り込み
- INTピンから信号が来る
- 種類
	- 自由落下
	- モーション検知
	- ゼロモーション検知
	- FIFOオーバーフロー
	- データレディ
	- i2cのエラーとか
