# MSER Algorithm Documentation

## Overview

Marginal Standard Error Rule (MSER) は、シミュレーション出力の定常状態検出に使用される統計手法です。White (1997) により提案され、シミュレーションの初期過渡期間を自動的に特定し、適切な統計解析のためのデータ切り捨て点を決定します。

## Mathematical Foundation

### 基本概念

シミュレーション出力の時系列データ `{Y₀, Y₁, ..., Yₙ₋₁}` に対して、MSERは以下の問題を解決します：

**目標**: 初期過渡期間の長さ `d̂(n)` を見つけ、定常状態データのみを統計解析に使用する

### MSER-1 (Original MSER)

White (1997) の原著論文に基づく標準的なMSER実装：

#### 定義

切り捨て点 `k` におけるMSER値：

```
gₙ(k) = Sₙ,ₖ² / (n-k)²
```

where:
- `n`: 総サンプル数
- `k`: 切り捨て点候補 (0 ≤ k ≤ ⌊n/2⌋-1)
- `Sₙ,ₖ²`: 切り捨て後データの平方和

#### 詳細計算

1. **切り捨て後サンプル平均**:
   ```
   Ȳₙ,ₖ = 1/(n-k) ∑ⱼ₌ₖⁿ⁻¹ Yⱼ
   ```

2. **平方偏差和**:
   ```
   Sₙ,ₖ² = ∑ⱼ₌ₖⁿ⁻¹ (Yⱼ - Ȳₙ,ₖ)²
   ```

3. **最適切り捨て点**:
   ```
   d̂(n) = argmin₀≤ₖ≤⌊ₙ/₂⌋₋₁ gₙ(k)
   ```

#### アルゴリズム

```cpp
double calculateMSERValue(const TimeSeriesData& data, size_t k) {
    size_t n = data.size();
    size_t effective_n = n - k;
    
    // 切り捨て後の平均値計算
    double mean = 0.0;
    for (size_t j = k; j < n; ++j) {
        mean += data[j];
    }
    mean /= effective_n;
    
    // 平方偏差和計算
    double sum_sq_dev = 0.0;
    for (size_t j = k; j < n; ++j) {
        double dev = data[j] - mean;
        sum_sq_dev += dev * dev;
    }
    
    // MSER値
    return sum_sq_dev / (effective_n * effective_n);
}
```

### MSER-m (Batched MSER)

計算効率向上のため、データをバッチに分割してバッチ平均に対してMSER-1を適用：

#### アルゴリズム

1. **バッチ分割**: 元データ `{Y₀, Y₁, ..., Yₙ₋₁}` をサイズ `m` のバッチに分割
   
2. **バッチ平均計算**:
   ```
   X̄ᵢ = 1/m ∑ⱼ₌ᵢₘ⁽ⁱ⁺¹⁾ᵐ⁻¹ Yⱼ,  i = 0, 1, ..., ⌊n/m⌋-1
   ```

3. **MSER-1適用**: バッチ平均系列 `{X̄₀, X̄₁, ..., X̄ₚ₋₁}` に対してMSER-1を実行

#### C++ Implementation

```cpp
TimeSeriesData createBatchMeans(const TimeSeriesData& data, size_t batchSize) {
    TimeSeriesData batchMeans;
    size_t numFullBatches = data.size() / batchSize;
    
    for (size_t i = 0; i < numFullBatches; ++i) {
        double batchSum = 0.0;
        size_t start = i * batchSize;
        size_t end = start + batchSize;
        
        for (size_t j = start; j < end; ++j) {
            batchSum += data[j];
        }
        
        batchMeans.push_back(batchSum / batchSize);
    }
    
    return batchMeans;
}
```

### MSER-5 (Industry Standard)

MSER-m の特別なケースで `m = 5` を使用：

#### 利点

1. **計算速度**: データサイズを1/5に削減
2. **実用性**: 多くの研究で有効性が確認済み
3. **バランス**: 精度と速度の良いトレードオフ

#### 推奨使用ケース

- 一般的なシミュレーション（Monte Carlo、Agent-based、等）
- リアルタイム定常状態検出
- 大規模データセットの高速処理

## Convergence Detection

### 基本判定ロジック

```cpp
bool isConverged(const MSERResult& result, double threshold) {
    return result.converged && result.mserValue <= threshold;
}
```

### 推奨収束閾値

| シミュレーション種類 | 推奨閾値 | 説明 |
|---------------------|----------|------|
| Monte Carlo | 0.01 | 標準的な精度 |
| 物理シミュレーション | 0.005 | より厳密な収束 |
| エージェントベース | 0.02 | 高速検出 |
| 金融モデリング | 0.001 | 高精度要求 |

### 実装例

```cpp
class ConvergenceDetector {
    double threshold;
    size_t minSamples;
    
public:
    bool checkConvergence(const TimeSeriesData& data) {
        if (data.size() < minSamples) return false;
        
        MSERResult result = mser.calculate(data);
        return result.converged && result.mserValue <= threshold;
    }
};
```

## Performance Optimization

### メモリ効率

1. **インクリメンタル計算**: 全履歴を保持せず、必要な統計量のみ更新
2. **循環バッファ**: 固定サイズバッファでメモリ使用量制限
3. **遅延評価**: チェック間隔を設けて計算頻度を制御

### 計算効率

1. **バッチ処理**: MSER-5使用で計算量を1/5に削減
2. **早期終了**: 明らかに収束していない場合の早期判定
3. **並列化**: 複数メトリック同時監視時の並列処理

### 実装例

```cpp
class EfficientMSER {
private:
    CircularBuffer<double> buffer;
    size_t checkInterval = 50;
    size_t lastCheck = 0;
    
public:
    bool addDataPoint(double value) {
        buffer.push(value);
        
        if (buffer.size() - lastCheck >= checkInterval) {
            lastCheck = buffer.size();
            return performMSERCheck();
        }
        
        return false;
    }
};
```

## Edge Cases and Robustness

### データ検証

1. **NaN/Inf チェック**: 不正な値の検出と処理
2. **最小サンプル数**: 統計的に意味のある分析のための下限
3. **分散ゼロ**: 定数データに対する特別処理

### エラーハンドリング

```cpp
MSERResult calculateRobustMSER(const TimeSeriesData& data) {
    MSERResult result;
    
    // データ検証
    if (!validateData(data)) {
        result.converged = false;
        result.reason = "Invalid data detected";
        return result;
    }
    
    // サンプル数チェック
    if (data.size() < MIN_SAMPLES) {
        result.converged = false;
        result.reason = "Insufficient samples";
        return result;
    }
    
    // 通常のMSER計算
    return performMSER(data);
}
```

## Theoretical Background

### 統計的根拠

MSER は以下の統計理論に基づいています：

1. **中心極限定理**: 大サンプルでの正規近似
2. **定常過程理論**: 定常状態での統計的性質
3. **最小二乗推定**: 分散の最小化による最適化

### 仮定

1. **弱定常性**: 平均と分散が時間に関して一定
2. **エルゴード性**: 時間平均と集合平均の一致
3. **有限分散**: 各観測値の分散が有限

### 限界

1. **非線形収束**: 振動的収束への対応限界
2. **多重定常状態**: 複数の定常状態が存在する場合
3. **短期記憶**: 長期依存性への感度不足

## Comparison with Other Methods

| 手法 | 精度 | 計算速度 | 実装複雑度 | 推奨用途 |
|------|------|----------|-----------|----------|
| MSER-1 | 高 | 低 | 低 | 高精度解析 |
| MSER-5 | 中-高 | 高 | 低 | 一般用途 |
| Welch's Method | 中 | 中 | 中 | 信号処理 |
| 移動平均 | 低 | 高 | 低 | 簡易監視 |

## References

1. White, K. P. Jr. (1997). The autoregressive model as a tool for steady-state detection. *ACM Transactions on Modeling and Computer Simulation*, 7(4), 459-478.

2. Oliveira, F.L.C., et al. (2024). Simulation output analysis using MSER-m rule. *Proceedings of the Winter Simulation Conference*.

3. Law, A. M. (2015). *Simulation Modeling and Analysis* (5th ed.). McGraw-Hill.

4. Banks, J., et al. (2010). *Discrete-Event System Simulation* (5th ed.). Prentice Hall.