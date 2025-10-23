# RoboCon2025
週10で松屋 チームの作品用リポジトリです

キャリブレーション用サイト
https://personal.qumcum.com/robo_adjust/

## フォルダ構成

```
RoboCon2025/
├── README.md              # このファイル
├── Production/            # 本番用コード
│   └── .gitkeep
├── Prototype/             # プロトタイプ開発
│   ├── Common/            # 共通ライブラリ・ユーティリティ
│   │   └── .gitkeep
│   ├── Kubota/            # 久保田のプロトタイプ
│   │   └── .gitkeep
│   ├── Masuda/            # 増田のプロトタイプ
│   │   └── .gitkeep
│   └── Nakagawa/          # 中川のプロトタイプ
│       └── .gitkeep
└── Samples/               # サンプルコード・テスト用
    ├── .gitkeep
    ├── Aquestalk.cpp      # 音声合成サンプル
    ├── Beep.cpp           # ビープ音出力サンプル
    └── RGB.cpp            # RGB LED制御サンプル
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