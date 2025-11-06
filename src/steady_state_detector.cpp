#include "mser/steady_state_detector.h"
#include <iostream>
#include <algorithm>

namespace mser {

SteadyStateDetector::SteadyStateDetector(const SteadyStateConfig& config)
    : config_(config), converged_(false), lastCheckIndex_(0) {
    mserCalculator_ = std::make_unique<MSER>();
    data_.reserve(config_.maxSamples);
}

SteadyStateDetector::~SteadyStateDetector() {
}

// ============================================================================
// リアルタイム検出機能の実装
// ============================================================================

bool SteadyStateDetector::addDataPoint(TimeSeriesValue value) {
    if (converged_) {
        return true;  // 既に収束済み
    }
    
    data_.push_back(value);
    
    // 最大サンプル数制限
    if (data_.size() > config_.maxSamples) {
        converged_ = true;
        lastResult_.converged = false;
        lastResult_.reason = "最大サンプル数に達しましたが収束しませんでした";
        triggerCallback(lastResult_);
        return false;
    }
    
    // 検査タイミングかどうかチェック
    if (shouldPerformCheck()) {
        return checkConvergence();
    }
    
    return false;
}

bool SteadyStateDetector::addDataPoints(const std::vector<TimeSeriesValue>& values) {
    for (const auto& value : values) {
        if (addDataPoint(value)) {
            return true;  // 収束検出
        }
    }
    return false;
}

bool SteadyStateDetector::checkConvergence() {
    if (converged_) {
        return true;
    }
    
    // ウォーミングアップ期間のチェック
    if (isInWarmingPeriod()) {
        return false;
    }
    
    // 最小サンプル数のチェック
    if (data_.size() < config_.minSamples) {
        return false;
    }
    
    // MSER計算実行
    lastResult_ = mserCalculator_->calculate(data_, config_);
    lastCheckIndex_ = data_.size();
    
    // 収束判定
    bool newlyConverged = evaluateConvergence(lastResult_);
    
    if (newlyConverged && !converged_) {
        converged_ = true;
        triggerCallback(lastResult_);
    }
    
    return converged_;
}

void SteadyStateDetector::reset() {
    data_.clear();
    converged_ = false;
    lastCheckIndex_ = 0;
    lastResult_ = MSERResult();
}

// ============================================================================
// 状態取得機能の実装
// ============================================================================

size_t SteadyStateDetector::getCurrentSampleCount() const {
    return data_.size();
}

const MSERResult& SteadyStateDetector::getLastResult() const {
    return lastResult_;
}

bool SteadyStateDetector::hasConverged() const {
    return converged_;
}

Statistics SteadyStateDetector::getCurrentStatistics() const {
    if (data_.empty()) {
        return Statistics();
    }
    
    return mserCalculator_->calculateStatistics(data_, 0, data_.size());
}

// ============================================================================
// 設定機能の実装
// ============================================================================

void SteadyStateDetector::updateConfig(const SteadyStateConfig& config) {
    config_ = config;
    
    // データ容量の調整
    if (data_.capacity() < config_.maxSamples) {
        data_.reserve(config_.maxSamples);
    }
}

void SteadyStateDetector::setConvergenceCallback(std::function<void(const MSERResult&)> callback) {
    convergenceCallback_ = callback;
}

// ============================================================================
// データアクセス機能の実装
// ============================================================================

TimeSeriesData SteadyStateDetector::getAccumulatedData() const {
    return data_;  // コピーを返す
}

double SteadyStateDetector::getCurrentMean() const {
    if (data_.empty()) {
        return 0.0;
    }
    
    Statistics stats = getCurrentStatistics();
    return stats.mean;
}

// ============================================================================
// 内部機能の実装
// ============================================================================

bool SteadyStateDetector::shouldPerformCheck() const {
    // ウォーミングアップ期間中はチェックしない
    if (isInWarmingPeriod()) {
        return false;
    }
    
    // 最小サンプル数未満はチェックしない
    if (data_.size() < config_.minSamples) {
        return false;
    }
    
    // チェック間隔に基づく判定
    size_t samplesSinceLastCheck = data_.size() - lastCheckIndex_;
    return samplesSinceLastCheck >= config_.checkInterval;
}

bool SteadyStateDetector::isInWarmingPeriod() const {
    if (!config_.enableWarming) {
        return false;
    }
    
    return data_.size() < config_.warmingSteps;
}

bool SteadyStateDetector::evaluateConvergence(const MSERResult& result) {
    if (!result.converged) {
        return false;
    }
    
    // MSER値が収束閾値以下かどうか
    return result.mserValue <= config_.convergenceThreshold;
}

void SteadyStateDetector::triggerCallback(const MSERResult& result) {
    if (convergenceCallback_) {
        convergenceCallback_(result);
    }
}

// ============================================================================
// 統合ヘルパー関数の実装
// ============================================================================

namespace integration {

std::unique_ptr<SteadyStateDetector> createForPhysXSimulation(
    const SteadyStateConfig& config) {
    SteadyStateConfig physxConfig = config;
    
    // PhysXシミュレーション向けのデフォルト調整
    if (physxConfig.variant == MSERVariant::MSER_1) {
        physxConfig.variant = MSERVariant::MSER_5;  // より高速なMSER-5を推奨
    }
    
    if (physxConfig.minSamples < 200) {
        physxConfig.minSamples = 200;  // PhysX では少し多めのサンプルを推奨
    }
    
    auto detector = std::make_unique<SteadyStateDetector>(physxConfig);
    
    // PhysX 用のコールバック設定（ログ出力）
    detector->setConvergenceCallback([](const MSERResult& result) {
        std::cout << "[定常状態検出] シミュレーション収束を検出" << std::endl;
        std::cout << "  切り捨て点: " << result.truncationPoint << std::endl;
        std::cout << "  MSER値: " << result.mserValue << std::endl;
        std::cout << "  総サンプル数: " << result.totalSamples << std::endl;
        std::cout << "  理由: " << result.reason << std::endl;
    });
    
    return detector;
}

std::unique_ptr<SteadyStateDetector> createForGenericSimulation(
    const std::string& metricName,
    const SteadyStateConfig& config) {
    auto detector = std::make_unique<SteadyStateDetector>(config);
    
    // 汎用シミュレーション用のコールバック設定
    detector->setConvergenceCallback([metricName](const MSERResult& result) {
        std::cout << "[定常状態検出] " << metricName << " が収束しました" << std::endl;
        std::cout << "  MSER変種: ";
        switch (result.variant) {
            case MSERVariant::MSER_1: std::cout << "MSER-1"; break;
            case MSERVariant::MSER_5: std::cout << "MSER-5"; break;
            case MSERVariant::MSER_M: std::cout << "MSER-m"; break;
        }
        std::cout << std::endl;
        std::cout << "  切り捨て点: " << result.truncationPoint << std::endl;
        std::cout << "  MSER値: " << result.mserValue << std::endl;
        std::cout << "  総サンプル数: " << result.totalSamples << std::endl;
        
        if (result.variant != MSERVariant::MSER_1) {
            std::cout << "  バッチ数: " << result.batchCount << std::endl;
        }
        
        std::cout << "  理由: " << result.reason << std::endl;
    });
    
    return detector;
}

} // namespace integration

} // namespace mser