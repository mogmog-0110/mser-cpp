#pragma once

#include "types.h"
#include <vector>
#include <cstddef>

namespace mser {

/**
 * MSER (Marginal Standard Error Rule) クラス
 * 
 * White (1997) 論文に基づくMSER実装
 * MSER-1（オリジナル）とMSER-5（業界標準）をサポート
 */
class MSER {
public:
    /**
     * コンストラクター
     */
    MSER();
    
    /**
     * デストラクター
     */
    ~MSER();

    // ============================================================================
    // MSER計算機能
    // ============================================================================
    
    /**
     * MSER-1計算（オリジナルMSER）
     * @param data 時系列データ
     * @return MSER計算結果
     */
    MSERResult calculateMSER1(const TimeSeriesData& data);
    
    /**
     * MSER-5計算（業界標準：バッチサイズ5）
     * @param data 時系列データ
     * @return MSER計算結果
     */
    MSERResult calculateMSER5(const TimeSeriesData& data);
    
    /**
     * MSER-m計算（任意バッチサイズ）
     * @param data 時系列データ
     * @param batchSize バッチサイズ
     * @return MSER計算結果
     */
    MSERResult calculateMSERm(const TimeSeriesData& data, size_t batchSize);
    
    /**
     * 自動MSER計算（設定に基づく）
     * @param data 時系列データ
     * @param config 設定
     * @return MSER計算結果
     */
    MSERResult calculate(const TimeSeriesData& data, const SteadyStateConfig& config);

    // ============================================================================
    // 統計計算機能
    // ============================================================================
    
    /**
     * 基本統計量計算
     * @param data データ
     * @param startIndex 開始インデックス
     * @param endIndex 終了インデックス（排他的）
     * @return 統計量
     */
    Statistics calculateStatistics(const TimeSeriesData& data, 
                                 size_t startIndex, 
                                 size_t endIndex);
    
    /**
     * バッチ統計計算
     * @param data 元データ
     * @param batchSize バッチサイズ
     * @return バッチ統計
     */
    BatchStatistics calculateBatchStatistics(const TimeSeriesData& data, 
                                            size_t batchSize);

    // ============================================================================
    // ヘルパー機能
    // ============================================================================
    
    /**
     * 最適な切り捨て点の検索
     * @param data データ
     * @param maxTruncation 最大切り捨て点
     * @return 切り捨て点とMSER値のペア
     */
    std::pair<size_t, double> findOptimalTruncationPoint(const TimeSeriesData& data);

private:
    // ============================================================================
    // 内部計算機能
    // ============================================================================
    
    /**
     * MSER値計算（White 1997の式）
     * gn(k) = Sn,k²/(n-k)² = 1/(n-k)² ∑j=k^(n-1) (Yj - Ȳn,k)²
     */
    double calculateMSERValue(const TimeSeriesData& data, 
                            size_t truncationPoint);
    
    /**
     * バッチ平均系列の生成
     */
    TimeSeriesData createBatchMeans(const TimeSeriesData& data, 
                                  size_t batchSize);
    
    /**
     * サンプル平均計算
     */
    double calculateMean(const TimeSeriesData& data, 
                       size_t startIndex, 
                       size_t endIndex);
    
    /**
     * データ検証
     */
    bool validateData(const TimeSeriesData& data, 
                     size_t minRequiredSize = 10);
};

} // namespace mser