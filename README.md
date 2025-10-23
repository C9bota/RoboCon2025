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
│   ├── Name1/             # プロトタイプ1
│   │   └── .gitkeep
│   ├── Name2/             # プロトタイプ2
│   │   └── .gitkeep
│   └── Name3/             # プロトタイプ3
│       └── .gitkeep
└── Samples/               # サンプルコード・テスト用
    └── .gitkeep
```

- **Production/**: 本番環境で使用する最終的なコード
- **Prototype/**: 開発中のプロトタイプコード
  - **Common/**: 各プロトタイプで共通して使用するライブラリやユーティリティ
  - **Name1~3/**: 各種プロトタイプの実装
- **Samples/**: 実験用のサンプルコードやテストコード