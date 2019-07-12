// 動作モード
enum class Mode : char {
	Wait,		// ウェイトモード．コマンド受領までなにもしないをする．
	Standby,	// スタンバイモード．離床判定を行う．
	Flight,		// フライトモード．離床〜開傘まで．
	Descent,	// ディセントモード．開傘〜着水まで．
};

// グローバル変数
Mode g_mode;

// 初期化関数．一度だけ実行される．
void setup(){
	//TODO: センサ初期化
	//TODO: TWE-Lite初期化
	//TODO: 動作モードをSDカードから読み込む
	// (動作中に瞬断して再起動する可能性がある)
	if(g_mode != Mode::Standby)
		return;		// 再起動時は早くそのモードの動作に戻る
	//TODO: 地上局に起動を通知
}

void loop(){
	switch(g_mode){
		case Mode::Wait:
			break;
		case Mode::Standby:
			break;
		case Mode::Flight:
			break;
		case Mode::Descent:
			break;
	}

	//TODO: テレメトリ送信
}
