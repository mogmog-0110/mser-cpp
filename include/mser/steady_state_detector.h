#pragma once

#include "mser.h"
#include "types.h"
#include <functional>
#include <memory>
#include <string>

namespace mser {

/**
 * 定常状態検出器
 * 
 * シミュレーション進行中にリアルタイムで定常状態を検出し、
 * 自動的にシミュレーションを終了する機能を提供
 */
class SteadyStateDetector {
public:
    /**
     * コンストラクター
     * @param config 検出設定
     */
    explicit SteadyStateDetector(const SteadyStateConfig& config = SteadyStateConfig());
    
    /**
     * デストラクター
     */
    ~SteadyStateDetector();

    // ============================================================================
    // リアルタイム検出機能
    // ============================================================================
    
    /**
     * データポイント追加
     * @param value 新しいデータ値
     * @return 定常状態に達した場合true
     */
    bool addDataPoint(TimeSeriesValue value);
    
    /**
     * 複数データポイント追加
     * @param values データ値の配列
     * @return 定常状態に達した場合true
     */
    bool addDataPoints(const std::vector<TimeSeriesValue>& values);
    
    /**
     * 強制検査実行
     * @return 現在の収束状態
     */
    bool checkConvergence();
    
    /**
     * 検出器リセット
     */
    void reset();

    // ============================================================================
    // 状態取得機能
    // ============================================================================
    
    /**
     * 現在のデータ数取得
     */
    size_t getCurrentSampleCount() const;
    
    /**
     * 最新のMSER結果取得
     */
    const MSERResult& getLastResult() const;
    
    /**
     * 収束状態取得
     */
    bool hasConverged() const;
    
    /**
     * 現在の統計量取得
     */
    Statistics getCurrentStatistics() const;

    // ============================================================================
    // 設定機能
    // ============================================================================
    
    /**
     * 設定更新
     */
    void updateConfig(const SteadyStateConfig& config);
    
    /**
     * コールバック設定（収束検出時に呼び出される）
     */
    void setConvergenceCallback(std::function<void(const MSERResult&)> callback);

    // ============================================================================
    // データアクセス機能
    // ============================================================================
    
    /**
     * 蓄積データ取得（コピー）
     */
    TimeSeriesData getAccumulatedData() const;
    
    /**
     * 現在の平均値取得
     */
    double getCurrentMean() const;

private:
    // ============================================================================
    // 内部状態
    // ============================================================================
    
    SteadyStateConfig config_;              // 検出設定
    TimeSeriesData data_;                   // 蓄積データ
    MSERResult lastResult_;                 // 最新結果
    bool converged_;                        // 収束フラグ
    size_t lastCheckIndex_;                 // 最後のチェック位置
    
    std::unique_ptr<MSER> mserCalculator_;  // MSER計算器
    std::function<void(const MSERResult&)> convergenceCallback_;  // コールバック
    
    // ============================================================================
    // 内部機能
    // ============================================================================
    
    /**
     * 検査タイミング判定
     */
    bool shouldPerformCheck() const;
    
    /**
     * ウォーミングアップ期間判定
     */
    bool isInWarmingPeriod() const;
    
    /**
     * 収束判定ロジック
     */
    bool evaluateConvergence(const MSERResult& result);
    
    /**
     * コールバック呼び出し
     */
    void triggerCallback(const MSERResult& result);
};

/**
 * シミュレーション統合用のヘルパー関数
 */
namespace integration {

/**
 * PhysXシミュレーション用の定常状態検出器作成
 * @param config 設定
 * @return 検出器のユニークポインタ
 */
std::unique_ptr<SteadyStateDetector> createForPhysXSimulation(
    const SteadyStateConfig& config = SteadyStateConfig());

/**
 * 汎用シミュレーション用の定常状態検出器作成
 * @param metricName 検出対象メトリック名
 * @param config 設定
 * @return 検出器のユニークポインタ
 */
std::unique_ptr<SteadyStateDetector> createForGenericSimulation(
    const std::string& metricName,
    const SteadyStateConfig& config = SteadyStateConfig());

} // namespace integration

} // namespace mser