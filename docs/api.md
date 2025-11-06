# API Reference

## Namespace: mser

すべてのMSERライブラリ機能は `mser` 名前空間内で定義されています。

## Core Classes

### MSER

メインのMSER計算クラス。

#### Constructor

```cpp
MSER();
```

基本的なMSER計算機能を提供するクラスのインスタンスを作成します。

#### Public Methods

##### calculateMSER1

```cpp
MSERResult calculateMSER1(const TimeSeriesData& data);
```

オリジナルのMSER-1アルゴリズムを実行します。

**Parameters:**
- `data`: 時系列データ（`std::vector<double>`）

**Returns:**
- `MSERResult`: 計算結果

**Example:**
```cpp
mser::MSER calculator;
std::vector<double> data = {1.0, 2.0, 3.0, /* ... */};
auto result = calculator.calculateMSER1(data);
```

##### calculateMSER5

```cpp
MSERResult calculateMSER5(const TimeSeriesData& data);
```

業界標準のMSER-5（バッチサイズ5）を実行します。

**Parameters:**
- `data`: 時系列データ

**Returns:**
- `MSERResult`: 計算結果

##### calculateMSERm

```cpp
MSERResult calculateMSERm(const TimeSeriesData& data, size_t batchSize);
```

任意のバッチサイズでMSER-mを実行します。

**Parameters:**
- `data`: 時系列データ
- `batchSize`: バッチサイズ（推奨: 3-10）

**Returns:**
- `MSERResult`: 計算結果

##### calculate

```cpp
MSERResult calculate(const TimeSeriesData& data, const SteadyStateConfig& config);
```

設定に基づいて適切なMSER変種を自動選択して実行します。

**Parameters:**
- `data`: 時系列データ
- `config`: 検出設定

**Returns:**
- `MSERResult`: 計算結果

##### calculateStatistics

```cpp
Statistics calculateStatistics(const TimeSeriesData& data, 
                             size_t startIndex, 
                             size_t endIndex);
```

指定範囲の基本統計量を計算します。

**Parameters:**
- `data`: データ
- `startIndex`: 開始インデックス（包含）
- `endIndex`: 終了インデックス（排他）

**Returns:**
- `Statistics`: 統計量（平均、分散、標準誤差）

---

### SteadyStateDetector

リアルタイム定常状態検出クラス。

#### Constructor

```cpp
explicit SteadyStateDetector(const SteadyStateConfig& config = SteadyStateConfig());
```

**Parameters:**
- `config`: 検出設定（オプション、デフォルト値使用可）

#### Public Methods

##### addDataPoint

```cpp
bool addDataPoint(TimeSeriesValue value);
```

新しいデータポイントを追加し、必要に応じて収束チェックを実行します。

**Parameters:**
- `value`: 新しいデータ値

**Returns:**
- `bool`: 定常状態に到達した場合 `true`

**Example:**
```cpp
mser::SteadyStateDetector detector;
for (int i = 0; i < 1000; ++i) {
    double value = runSimulation();
    if (detector.addDataPoint(value)) {
        std::cout << "Converged at step " << i << std::endl;
        break;
    }
}
```

##### addDataPoints

```cpp
bool addDataPoints(const std::vector<TimeSeriesValue>& values);
```

複数のデータポイントを一度に追加します。

**Parameters:**
- `values`: データ値の配列

**Returns:**
- `bool`: 定常状態に到達した場合 `true`

##### checkConvergence

```cpp
bool checkConvergence();
```

現在蓄積されているデータに対して強制的に収束チェックを実行します。

**Returns:**
- `bool`: 収束している場合 `true`

##### reset

```cpp
void reset();
```

検出器を初期状態にリセットします。蓄積データと状態をクリアします。

##### getCurrentSampleCount

```cpp
size_t getCurrentSampleCount() const;
```

現在蓄積されているサンプル数を取得します。

**Returns:**
- `size_t`: サンプル数

##### getLastResult

```cpp
const MSERResult& getLastResult() const;
```

最後に実行されたMSER計算の結果を取得します。

**Returns:**
- `const MSERResult&`: 最新の計算結果

##### hasConverged

```cpp
bool hasConverged() const;
```

収束状態を確認します。

**Returns:**
- `bool`: 収束している場合 `true`

##### getCurrentStatistics

```cpp
Statistics getCurrentStatistics() const;
```

現在蓄積されているデータの統計量を取得します。

**Returns:**
- `Statistics`: 現在の統計量

##### updateConfig

```cpp
void updateConfig(const SteadyStateConfig& config);
```

検出設定を更新します。

**Parameters:**
- `config`: 新しい設定

##### setConvergenceCallback

```cpp
void setConvergenceCallback(std::function<void(const MSERResult&)> callback);
```

収束検出時に呼び出されるコールバック関数を設定します。

**Parameters:**
- `callback`: コールバック関数

**Example:**
```cpp
detector.setConvergenceCallback([](const mser::MSERResult& result) {
    std::cout << "Converged! MSER value: " << result.mserValue << std::endl;
    stopSimulation();
});
```

##### getAccumulatedData

```cpp
TimeSeriesData getAccumulatedData() const;
```

蓄積されたすべてのデータのコピーを取得します。

**Returns:**
- `TimeSeriesData`: データのコピー

##### getCurrentMean

```cpp
double getCurrentMean() const;
```

現在のデータの平均値を取得します。

**Returns:**
- `double`: 平均値

## Data Structures

### MSERResult

MSER計算の結果を格納する構造体。

```cpp
struct MSERResult {
    size_t truncationPoint;     // 最適切り捨て点
    double mserValue;           // MSER値
    bool converged;             // 収束判定
    std::string reason;         // 判定理由
    size_t totalSamples;        // 総サンプル数
    size_t batchCount;          // バッチ数（MSER-m用）
    MSERVariant variant;        // 使用したMSER変種
};
```

#### Members

- **truncationPoint**: 最適な切り捨て点 `d̂(n)`
- **mserValue**: 計算されたMSER値 `gₙ(d̂)`
- **converged**: 計算が成功し、有効な結果が得られたかどうか
- **reason**: 計算結果の説明（エラーメッセージなど）
- **totalSamples**: 入力データの総サンプル数
- **batchCount**: バッチ処理時のバッチ数（MSER-1の場合は0）
- **variant**: 使用されたMSER変種

### SteadyStateConfig

定常状態検出の設定を格納する構造体。

```cpp
struct SteadyStateConfig {
    MSERVariant variant = MSERVariant::MSER_5;
    size_t batchSize = 5;
    size_t minSamples = 100;
    size_t maxSamples = 10000;
    double convergenceThreshold = 0.01;
    size_t checkInterval = 50;
    bool enableWarming = true;
    size_t warmingSteps = 50;
};
```

#### Members

- **variant**: 使用するMSER変種（MSER_1, MSER_5, MSER_M）
- **batchSize**: MSER-m使用時のバッチサイズ
- **minSamples**: 検出開始に必要な最小サンプル数
- **maxSamples**: 最大蓄積サンプル数（メモリ制限）
- **convergenceThreshold**: 収束判定閾値
- **checkInterval**: 収束チェックの実行間隔
- **enableWarming**: ウォーミングアップ期間の有効化
- **warmingSteps**: ウォーミングアップステップ数

### Statistics

基本統計量を格納する構造体。

```cpp
struct Statistics {
    double mean;            // 平均値
    double variance;        // 分散
    double standardError;   // 標準誤差
    size_t sampleCount;     // サンプル数
};
```

### BatchStatistics

バッチ処理の統計情報を格納する構造体。

```cpp
struct BatchStatistics {
    TimeSeriesData batchMeans;  // バッチ平均の系列
    size_t originalSampleCount; // 元データのサンプル数
    size_t batchSize;           // バッチサイズ
};
```

## Enumerations

### MSERVariant

使用可能なMSER変種の列挙型。

```cpp
enum class MSERVariant {
    MSER_1,     // オリジナルMSER
    MSER_5,     // バッチサイズ5（業界標準）
    MSER_M      // 任意バッチサイズ
};
```

## Type Aliases

### TimeSeriesValue

```cpp
using TimeSeriesValue = double;
```

時系列データの単一値の型。

### TimeSeriesData

```cpp
using TimeSeriesData = std::vector<TimeSeriesValue>;
```

時系列データの配列型。

## Integration Namespace

### integration::createForPhysXSimulation

```cpp
std::unique_ptr<SteadyStateDetector> createForPhysXSimulation(
    const SteadyStateConfig& config = SteadyStateConfig());
```

PhysXシミュレーション向けに最適化された検出器を作成します。

**Parameters:**
- `config`: 設定（オプション）

**Returns:**
- `std::unique_ptr<SteadyStateDetector>`: 最適化された検出器

**Example:**
```cpp
auto detector = mser::integration::createForPhysXSimulation();
// PhysX最適化済みの設定で検出器が作成される
```

### integration::createForGenericSimulation

```cpp
std::unique_ptr<SteadyStateDetector> createForGenericSimulation(
    const std::string& metricName,
    const SteadyStateConfig& config = SteadyStateConfig());
```

汎用シミュレーション向けの検出器を作成します。

**Parameters:**
- `metricName`: 監視メトリック名（ログ用）
- `config`: 設定（オプション）

**Returns:**
- `std::unique_ptr<SteadyStateDetector>`: 汎用検出器

## Usage Patterns

### 基本パターン

```cpp
#include <mser/steady_state_detector.h>

// 1. 設定作成
mser::SteadyStateConfig config;
config.variant = mser::MSERVariant::MSER_5;
config.convergenceThreshold = 0.01;

// 2. 検出器作成
auto detector = std::make_unique<mser::SteadyStateDetector>(config);

// 3. データ追加とチェック
for (int step = 0; step < maxSteps; ++step) {
    double value = runSimulation();
    if (detector->addDataPoint(value)) {
        break; // 収束
    }
}
```

### コールバックパターン

```cpp
detector->setConvergenceCallback([&](const mser::MSERResult& result) {
    std::cout << "Simulation converged!" << std::endl;
    std::cout << "Final MSER value: " << result.mserValue << std::endl;
    simulationRunning = false; // フラグ設定で停止
});
```

### バッチ処理パターン

```cpp
std::vector<double> batch_data;
for (int i = 0; i < batch_size; ++i) {
    batch_data.push_back(runSimulation());
}

if (detector->addDataPoints(batch_data)) {
    // バッチ処理中に収束検出
}
```

## Error Handling

### 一般的なエラー

1. **データ不足**: `minSamples` 未満のデータでの計算試行
2. **無効データ**: NaN または Inf を含むデータ
3. **設定エラー**: 不正な設定値（負のバッチサイズなど）

### エラーチェック例

```cpp
auto result = mser.calculateMSER5(data);
if (!result.converged) {
    std::cerr << "MSER calculation failed: " << result.reason << std::endl;
    // エラー処理
}
```

## Performance Considerations

### メモリ使用量

- `SteadyStateDetector` は設定された `maxSamples` 分のメモリを予約
- 大量データの場合は `maxSamples` を適切に設定

### 計算頻度

- `checkInterval` を調整して計算頻度を制御
- 頻繁なチェックは性能に影響

### バッチサイズ選択

| バッチサイズ | メモリ削減 | 精度 | 推奨用途 |
|-------------|-----------|------|----------|
| 3 | 67% | 高 | 高精度要求 |
| 5 | 80% | 中-高 | 汎用（推奨） |
| 10 | 90% | 中 | 高速処理 |