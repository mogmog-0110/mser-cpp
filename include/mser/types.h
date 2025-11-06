#pragma once

#include <vector>
#include <cstddef>

namespace mser {

/**
 * 時系列データ点の型定義
 */
using TimeSeriesValue = double;
using TimeSeriesData = std::vector<TimeSeriesValue>;

/**
 * MSER変種の種類
 */
enum class MSERVariant {
    MSER_1,     // オリジナルMSER (全データ使用)
    MSER_5,     // バッチサイズ5のMSER-m (業界標準)
    MSER_M      // 任意バッチサイズのMSER-m
};

/**
 * MSER計算結果
 */
struct MSERResult {
    size_t truncationPoint;     // 最適切り捨て点 d̂(n)
    double mserValue;           // MSER値 gn(d̂)
    bool converged;             // 収束判定
    std::string reason;         // 判定理由
    size_t totalSamples;        // 総サンプル数
    size_t batchCount;          // バッチ数（MSER-m用）
    MSERVariant variant;        // 使用したMSER変種
    
    MSERResult() : truncationPoint(0), mserValue(0.0), converged(false), 
                   totalSamples(0), batchCount(0), variant(MSERVariant::MSER_5) {}
};

/**
 * 定常状態検出設定
 */
struct SteadyStateConfig {
    MSERVariant variant = MSERVariant::MSER_5;  // 使用するMSER変種
    size_t batchSize = 5;                       // バッチサイズ（MSER-m用）
    size_t minSamples = 100;                    // 最小サンプル数
    size_t maxSamples = 10000;                  // 最大サンプル数
    double convergenceThreshold = 0.01;         // 収束閾値
    size_t checkInterval = 50;                  // チェック間隔
    bool enableWarming = true;                  // ウォーミングアップ有効化
    size_t warmingSteps = 50;                   // ウォーミングアップステップ数
    
    SteadyStateConfig() = default;
};

/**
 * 統計情報
 */
struct Statistics {
    double mean;            // 平均値
    double variance;        // 分散
    double standardError;   // 標準誤差
    size_t sampleCount;     // サンプル数
    
    Statistics() : mean(0.0), variance(0.0), standardError(0.0), sampleCount(0) {}
};

/**
 * バッチ統計（MSER-m用）
 */
struct BatchStatistics {
    TimeSeriesData batchMeans;  // バッチ平均の系列
    size_t originalSampleCount; // 元データのサンプル数
    size_t batchSize;           // バッチサイズ
    
    BatchStatistics() : originalSampleCount(0), batchSize(0) {}
};

} // namespace mser