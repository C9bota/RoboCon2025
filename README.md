# RoboCon2025
週10で松屋 チームの作品用リポジトリです

キャリブレーション用サイト
https://personal.qumcum.com/robo_adjust/

## フォルダ構成
Arduino IDEは、スケッチフォルダ直下にあるファイルおよび、srcフォルダの中をコンパイル対象とするため、以下のようなフォルダ構成とする。
各ライブラリは機能ごとにフォルダを分け、src配下へ置くこと。

```
RoboCon2025/
├── README.md             
├── Production/            # 本番用コード
│   └── qumcum_fes_2025/   # スケッチフォルダ
│       ├── qumcum_fes_2025.ino   # エントリポイント
│       └── src/
│           ├── core/      
│           ├── midi/      # MIDI読み込み
│           ├── sd/        # 
│           └── servo/     # サーボ駆動ライブラリ
│               ├── lib/
│               └── src/
│
├── Prototype/             # プロトタイプ開発
│   ├── Common/            # 共通ライブラリ・ユーティリティ
│   │   └── midifiles      # MIDIファイル
│   ├── Kubota/           
│   ├── Masuda/           
│   └── Nakagawa/          
|
└── Samples/               # Qumcum公式サンプルコード
    ├── Aquestalk.cpp
    ├── Beep.cpp
    ├── RGB.cpp
    └── Servo.cpp

```

- **Production/**: 本番環境で使用する最終的なコード
- **Prototype/**: 開発中のプロトタイプコード
  - **Common/**: 各プロトタイプで共通して使用するライブラリやユーティリティ
  - **Kubota/**: 久保田のプロトタイプ実装
  - **Masuda/**: 増田のプロトタイプ実装
  - **Nakagawa/**: 中川のプロトタイプ実装
- **Samples/**: 実験用のサンプルコードやテストコード
  - [`Aquestalk.cpp`](Samples/Aquestalk.cpp): AquesTalkを使用した音声合成のサンプル
  - [`Beep.cpp`](Samples/Beep.cpp): PWMを使用したビープ音出力のサンプル
  - [`RGB.cpp`](Samples/RGB.cpp): RGB LEDの制御サンプル