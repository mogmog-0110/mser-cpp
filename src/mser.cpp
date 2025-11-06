#include "mser/mser.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace mser {

MSER::MSER() {
}

MSER::~MSER() {
}

// ============================================================================
// MSER計算機能の実装
// ============================================================================

MSERResult MSER::calculateMSER1(const TimeSeriesData& data) {
    MSERResult result;
    result.variant = MSERVariant::MSER_1;
    result.totalSamples = data.size();
    
    if (!validateData(data)) {
        result.converged = false;
        result.reason = "データが不十分または無効です";
        return result;
    }
    
    auto [truncPoint, mserVal] = findOptimalTruncationPoint(data);
    
    result.truncationPoint = truncPoint;
    result.mserValue = mserVal;
    result.converged = (mserVal < std::numeric_limits<double>::infinity());
    result.reason = result.converged ? "収束しました" : "収束しませんでした";
    
    return result;
}

MSERResult MSER::calculateMSER5(const TimeSeriesData& data) {
    return calculateMSERm(data, 5);  // 業界標準のバッチサイズ5
}

MSERResult MSER::calculateMSERm(const TimeSeriesData& data, size_t batchSize) {
    MSERResult result;
    result.variant = (batchSize == 5) ? MSERVariant::MSER_5 : MSERVariant::MSER_M;
    result.totalSamples = data.size();
    
    if (!validateData(data, batchSize * 2)) {  // バッチ処理には最低限のサンプル数が必要
        result.converged = false;
        result.reason = "バッチ処理に必要なデータが不十分です";
        return result;
    }
    
    // バッチ平均系列の作成
    TimeSeriesData batchMeans = createBatchMeans(data, batchSize);
    result.batchCount = batchMeans.size();
    
    if (batchMeans.size() < 10) {  // 最低限のバッチ数
        result.converged = false;
        result.reason = "バッチ数が不十分です";
        return result;
    }
    
    // バッチ平均系列に対してMSER-1を適用
    auto [truncPoint, mserVal] = findOptimalTruncationPoint(batchMeans);
    
    result.truncationPoint = truncPoint;
    result.mserValue = mserVal;
    result.converged = (mserVal < std::numeric_limits<double>::infinity());
    result.reason = result.converged ? "収束しました" : "収束しませんでした";
    
    return result;
}

MSERResult MSER::calculate(const TimeSeriesData& data, const SteadyStateConfig& config) {
    switch (config.variant) {
        case MSERVariant::MSER_1:
            return calculateMSER1(data);
        case MSERVariant::MSER_5:
            return calculateMSER5(data);
        case MSERVariant::MSER_M:
            return calculateMSERm(data, config.batchSize);
        default:
            return calculateMSER5(data);  // デフォルトは業界標準のMSER-5
    }
}

// ============================================================================
// 統計計算機能の実装
// ============================================================================

Statistics MSER::calculateStatistics(const TimeSeriesData& data, 
                                    size_t startIndex, 
                                    size_t endIndex) {
    Statistics stats;
    
    if (startIndex >= endIndex || endIndex > data.size()) {
        return stats;  // 無効な範囲の場合はゼロ統計を返す
    }
    
    size_t n = endIndex - startIndex;
    stats.sampleCount = n;
    
    // 平均値計算
    stats.mean = calculateMean(data, startIndex, endIndex);
    
    // 分散計算
    double sumSquaredDeviations = 0.0;
    for (size_t i = startIndex; i < endIndex; ++i) {
        double deviation = data[i] - stats.mean;
        sumSquaredDeviations += deviation * deviation;
    }
    
    if (n > 1) {
        stats.variance = sumSquaredDeviations / (n - 1);
        stats.standardError = std::sqrt(stats.variance / n);
    }
    
    return stats;
}

BatchStatistics MSER::calculateBatchStatistics(const TimeSeriesData& data, 
                                              size_t batchSize) {
    BatchStatistics batchStats;
    batchStats.originalSampleCount = data.size();
    batchStats.batchSize = batchSize;
    batchStats.batchMeans = createBatchMeans(data, batchSize);
    
    return batchStats;
}

// ============================================================================
// ヘルパー機能の実装
// ============================================================================

std::pair<size_t, double> MSER::findOptimalTruncationPoint(const TimeSeriesData& data) {
    size_t n = data.size();
    size_t maxK = n / 2;  // White (1997): k ≤ ⌊n/2⌋-1
    
    if (maxK < 2) {
        return {0, std::numeric_limits<double>::infinity()};
    }
    
    double minMSER = std::numeric_limits<double>::infinity();
    size_t optimalK = 0;
    
    // d̂(n) = argmin[0≤k≤⌊n/2⌋-1] gn(k)
    for (size_t k = 0; k < maxK; ++k) {
        double mser = calculateMSERValue(data, k);
        
        if (mser < minMSER) {
            minMSER = mser;
            optimalK = k;
        }
    }
    
    return {optimalK, minMSER};
}

// ============================================================================
// 内部計算機能の実装
// ============================================================================

double MSER::calculateMSERValue(const TimeSeriesData& data, size_t truncationPoint) {
    size_t n = data.size();
    
    if (truncationPoint >= n || n - truncationPoint < 2) {
        return std::numeric_limits<double>::infinity();
    }
    
    // White (1997)の式: gn(k) = Sn,k²/(n-k)²
    // Sn,k² = ∑j=k^(n-1) (Yj - Ȳn,k)²
    
    size_t effectiveN = n - truncationPoint;
    
    // Ȳn,k の計算（切り捨て後の平均）
    double mean = calculateMean(data, truncationPoint, n);
    
    // Sn,k² の計算（切り捨て後の平方和）
    double sumSquaredDeviations = 0.0;
    for (size_t j = truncationPoint; j < n; ++j) {
        double deviation = data[j] - mean;
        sumSquaredDeviations += deviation * deviation;
    }
    
    // gn(k) = Sn,k²/(n-k)² の計算
    double mser = sumSquaredDeviations / (effectiveN * effectiveN);
    
    return mser;
}

TimeSeriesData MSER::createBatchMeans(const TimeSeriesData& data, size_t batchSize) {
    TimeSeriesData batchMeans;
    
    if (batchSize == 0) {
        return batchMeans;
    }
    
    size_t numFullBatches = data.size() / batchSize;
    batchMeans.reserve(numFullBatches);
    
    for (size_t i = 0; i < numFullBatches; ++i) {
        double batchSum = 0.0;
        size_t startIdx = i * batchSize;
        size_t endIdx = startIdx + batchSize;
        
        for (size_t j = startIdx; j < endIdx; ++j) {
            batchSum += data[j];
        }
        
        batchMeans.push_back(batchSum / batchSize);
    }
    
    return batchMeans;
}

double MSER::calculateMean(const TimeSeriesData& data, size_t startIndex, size_t endIndex) {
    if (startIndex >= endIndex || endIndex > data.size()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (size_t i = startIndex; i < endIndex; ++i) {
        sum += data[i];
    }
    
    return sum / (endIndex - startIndex);
}

bool MSER::validateData(const TimeSeriesData& data, size_t minRequiredSize) {
    if (data.size() < minRequiredSize) {
        return false;
    }
    
    // NaN や Inf のチェック
    for (const auto& value : data) {
        if (!std::isfinite(value)) {
            return false;
        }
    }
    
    return true;
}

} // namespace mser