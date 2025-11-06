# MSER-CPP: C++ Implementation of Marginal Standard Error Rule

[![CMake](https://github.com/shiggy/mser-cpp/workflows/CMake/badge.svg)](https://github.com/mogmog-0110/mser-cpp/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

MSERライブラリは、シミュレーションの定常状態検出のためのC++実装です。White (1997)論文に基づくMarginal Standard Error Rule (MSER)アルゴリズムを提供し、特に業界標準として広く使用されているMSER-5をサポートしています。

## 📋 Features

- **MSER-1**: オリジナルのMSER実装（全データ使用）
- **MSER-5**: 業界標準のバッチサイズ5版（推奨）
- **MSER-m**: 任意バッチサイズ版
- **リアルタイム検出**: シミュレーション進行中の自動的な定常状態検出
- **PhysXサポート**: 物理シミュレーション向けの最適化
- **C++17対応**: モダンC++機能を活用
- **CMake統合**: 簡単な依存関係管理とビルド

## 🚀 Quick Start

### 基本的な使用方法

```cpp
#include <mser/steady_state_detector.h>
#include <vector>

int main() {
    // MSER-5を使用した定常状態検出器を作成
    mser::SteadyStateConfig config;
    config.variant = mser::MSERVariant::MSER_5;
    config.convergenceThreshold = 0.01;
    
    auto detector = std::make_unique<mser::SteadyStateDetector>(config);
    
    // シミュレーションループ
    std::vector<double> simulationData;
    for (int step = 0; step < 10000; ++step) {
        double value = runSimulationStep();  // あなたのシミュレーション
        
        if (detector->addDataPoint(value)) {
            std::cout << "定常状態に到達しました！ステップ: " << step << std::endl;
            break;
        }
    }
    
    // 結果の確認
    const auto& result = detector->getLastResult();
    std::cout << "MSER値: " << result.mserValue << std::endl;
    std::cout << "切り捨て点: " << result.truncationPoint << std::endl;
    
    return 0;
}
```

### PhysXシミュレーション向け

```cpp
#include <mser/steady_state_detector.h>

// PhysX最適化設定で検出器を作成
auto detector = mser::integration::createForPhysXSimulation();

// 結合数やクラスタサイズなどのメトリックを監視
detector->setConvergenceCallback([](const mser::MSERResult& result) {
    std::cout << "PhysXシミュレーション収束検出" << std::endl;
    // 自動的にシミュレーション終了
    stopSimulation();
});
```

## 📦 Installation

### CMakeプロジェクトでの使用

1. プロジェクトをクローン：

```bash
git clone https://github.com/mogmog-0110/mser-cpp.git
cd mser-cpp
```

2. ビルド：

```bash
mkdir build && cd build
cmake ..
make
```

3. インストール：

```bash
sudo make install
```

### CMakeでの依存関係管理

既存のCMakeプロジェクトに統合する場合：

```cmake
find_package(mser-cpp REQUIRED)
target_link_libraries(your_target mser)
```

サブモジュールとして使用する場合：

```cmake
add_subdirectory(third_party/mser-cpp)
target_link_libraries(your_target mser)
```

## 🔧 Configuration

### 基本設定

```cpp
mser::SteadyStateConfig config;
config.variant = mser::MSERVariant::MSER_5;     // 使用するMSER変種
config.batchSize = 5;                           // バッチサイズ（MSER-m用）
config.minSamples = 100;                        // 最小サンプル数
config.maxSamples = 10000;                      // 最大サンプル数
config.convergenceThreshold = 0.01;             // 収束閾値
config.checkInterval = 50;                      // チェック間隔
config.enableWarming = true;                    // ウォーミングアップ有効化
config.warmingSteps = 50;                       // ウォーミングアップステップ数
```

### 推奨設定

| 用途 | バリアント | バッチサイズ | 収束閾値 | 最小サンプル | 説明 |
|------|------------|--------------|----------|--------------|------|
| 一般シミュレーション | MSER-5 | 5 | 0.01 | 100 | 業界標準 |
| 高精度解析 | MSER-1 | - | 0.005 | 200 | より厳密 |
| 高速検出 | MSER-5 | 5 | 0.02 | 50 | 速やかに終了 |
| PhysXシミュレーション | MSER-5 | 5 | 0.01 | 200 | 物理シミュレーション最適化 |

## 📚 Documentation

詳細なドキュメントは[docs/](docs/)ディレクトリにあります：

- [アルゴリズム詳細](docs/algorithm.md) - MSER理論と実装の詳細
- [API リファレンス](docs/api.md) - 完全なAPI仕様
- [論文・参考文献](docs/references.md) - 引用情報と関連研究

## 🧪 Examples

[examples/](examples/)ディレクトリにサンプルコードがあります：

- `basic_usage.cpp` - 基本的な使用方法
- `physx_integration.cpp` - PhysXとの統合例
- `custom_metrics.cpp` - カスタムメトリック監視
- `batch_comparison.cpp` - MSER変種の比較

## 🔬 Algorithm

このライブラリは以下の論文に基づく実装です：

### White (1997) MSER Algorithm

MSER値の計算式：

```
gn(k) = Sn,k² / (n-k)²
```

where:
- `Sn,k²` = 1/(n-k) ∑[j=k to n-1] (Yj - Ȳn,k)²
- `Ȳn,k` = 1/(n-k) ∑[j=k to n-1] Yj
- `d̂(n)` = argmin[0≤k≤⌊n/2⌋-1] gn(k)

### MSER変種

1. **MSER-1** (Original): 全データポイントを使用
2. **MSER-m** (Batched): データをサイズmのバッチに分割してバッチ平均に適用
3. **MSER-5** (Industry Standard): m=5のMSER-m、業界標準として広く使用

## 🤝 Contributing

1. フォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/amazing-feature`)
3. コミット (`git commit -m 'Add amazing feature'`)
4. プッシュ (`git push origin feature/amazing-feature`)
5. Pull Request作成

## 📄 License

このプロジェクトはMITライセンスの下で配布されています。詳細は[LICENSE](LICENSE)ファイルを参照してください。

## 📖 Citation

学術論文でこのライブラリを使用する場合は、以下を引用してください：

### Primary Reference (MSER Theory)
```bibtex
@article{white1997,
    title={The autoregressive model as a tool for steady-state detection},
    author={White, Kenneth P. Jr.},
    journal={ACM Transactions on Modeling and Computer Simulation},
    volume={7},
    number={4},
    pages={459--478},
    year={1997},
    publisher={ACM}
}
```

### MSER-m and Industry Applications
```bibtex
@inproceedings{oliveira2024,
    title={Simulation output analysis using MSER-m rule},
    author={Oliveira, F.L.C. and others},
    booktitle={Proceedings of the Winter Simulation Conference},
    pages={1--12},
    year={2024}
}
```

### This Implementation
```bibtex
@software{mser_cpp_2025,
    title={MSER-CPP: C++ Implementation of Marginal Standard Error Rule},
    author={Shiggy},
    year={2025},
    url={https://github.com/mogmog-0110/mser-cpp}
}
```

## 💬 Support

- 🐛 バグ報告: [GitHub Issues](https://github.com/shiggy/mser-cpp/issues)

**開発者**: Shiggy  
**プロジェクト**: マクロサイクル化シミュレーション自動終了システム  
**バージョン**: 1.0.0  
**最終更新**: 2025年11月