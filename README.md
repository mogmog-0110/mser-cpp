# mser-cpp

シミュレーションが定常状態に達したかを自動判定する C++ ライブラリです。White (1997) の Marginal Standard Error Rule（MSER）と、業界標準版の MSER-5 を実装しています。マクロサイクル化シミュレーションの終了判定を目的に開発しました。

[![CMake](https://github.com/mogmog-0110/mser-cpp/workflows/CMake/badge.svg)](https://github.com/mogmog-0110/mser-cpp/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 機能

- 全データ点を使うオリジナルの MSER-1
- バッチサイズを 5 とする、実務で標準的な MSER-5
- 任意のバッチサイズを指定できる MSER-m
- データ点を追加するたびに収束を確認する逐次判定
- PhysX 向けの既定値セット
- C++17 / CMake

## 使い方

```cpp
#include <mser/steady_state_detector.h>
#include <vector>

int main() {
    mser::SteadyStateConfig config;
    config.variant = mser::MSERVariant::MSER_5;
    config.convergenceThreshold = 0.01;

    auto detector = std::make_unique<mser::SteadyStateDetector>(config);

    for (int step = 0; step < 10000; ++step) {
        double value = runSimulationStep();

        if (detector->addDataPoint(value)) {
            std::cout << "定常状態に到達 step=" << step << std::endl;
            break;
        }
    }

    const auto& result = detector->getLastResult();
    std::cout << "MSER値: " << result.mserValue << std::endl;
    std::cout << "切り捨て点: " << result.truncationPoint << std::endl;
    return 0;
}
```

PhysX シミュレーションでは、既定値を設定する生成関数を利用できます。

```cpp
auto detector = mser::integration::createForPhysXSimulation();

detector->setConvergenceCallback([](const mser::MSERResult& result) {
    stopSimulation();
});
```

## ビルド

```bash
git clone https://github.com/mogmog-0110/mser-cpp.git
cd mser-cpp
mkdir build && cd build
cmake ..
make
```

既存の CMake プロジェクトには、`find_package` またはサブモジュールで追加します。

```cmake
find_package(mser-cpp REQUIRED)
target_link_libraries(your_target mser)
```

```cmake
add_subdirectory(third_party/mser-cpp)
target_link_libraries(your_target mser)
```

## 設定

```cpp
mser::SteadyStateConfig config;
config.variant = mser::MSERVariant::MSER_5;     // 使う変種
config.batchSize = 5;                           // バッチサイズ（MSER-m 用）
config.minSamples = 100;                        // 最小サンプル数
config.maxSamples = 10000;                      // 最大サンプル数
config.convergenceThreshold = 0.01;             // 収束閾値
config.checkInterval = 50;                      // チェック間隔
config.enableWarming = true;                    // ウォーミングアップ
config.warmingSteps = 50;                       // ウォーミングアップのステップ数
```

用途ごとの設定目安は次のとおりです。

| 用途 | 変種 | バッチサイズ | 収束閾値 | 最小サンプル |
|---|---|---:|---:|---:|
| 一般 | MSER-5 | 5 | 0.01 | 100 |
| 高精度 | MSER-1 | なし | 0.005 | 200 |
| 速く切り上げる | MSER-5 | 5 | 0.02 | 50 |
| PhysX | MSER-5 | 5 | 0.01 | 200 |

## アルゴリズム

打ち切り点 `k` には、切り捨て後の標本平均に対する周辺信頼区間の幅が最小となる位置を選びます。

```text
gn(k) = Sn,k² / (n-k)²

Sn,k² = 1/(n-k) ∑[j=k to n-1] (Yj - Ȳn,k)²
Ȳn,k  = 1/(n-k) ∑[j=k to n-1] Yj
d̂(n)  = argmin[0≤k≤⌊n/2⌋-1] gn(k)
```

MSER-m では、データをサイズ `m` のバッチに分割し、そのバッチ平均に同じ規則を適用します。`m = 5` が MSER-5 です。

詳しい説明は [docs/algorithm.md](docs/algorithm.md)、API は [docs/api.md](docs/api.md) を参照してください。使用例は [examples/](examples/) にあります。

## 参考文献

K. Preston White Jr., "An Effective Truncation Heuristic for Bias Reduction in
Simulation Output," *Simulation*, 69(6), 323–334, 1997.

```bibtex
@article{white1997,
    title   = {An Effective Truncation Heuristic for Bias Reduction in Simulation Output},
    author  = {White, K. Preston, Jr.},
    journal = {Simulation},
    volume  = {69},
    number  = {6},
    pages   = {323--334},
    year    = {1997}
}
```

## ライセンス

MIT。詳細は [LICENSE](LICENSE) を参照してください。
