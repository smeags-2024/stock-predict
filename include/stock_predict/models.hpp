#pragma once

#include <Eigen/Dense>
#include <memory>
#include <vector>
#include <functional>
#include <random>
#include "stock_predictor.hpp"

namespace stock_predict {

/**
 * @brief Simple mathematical stock predictor using linear regression and technical indicators
 */
class SimplePredictor : public IStockPredictor {
public:
    SimplePredictor(const std::string& symbol, const PredictorConfig& config);
    ~SimplePredictor() override = default;

    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<MarketData>& data, int epochs = 100) override;
    PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) override;
    std::vector<PredictionResult> predict_multi_day(const std::vector<MarketData>& recent_data, int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

private:
    std::string symbol_;
    PredictorConfig config_;
    bool is_trained_;
    double training_accuracy_;
    
    // Linear regression parameters
    Eigen::VectorXd weights_;
    double bias_;
    
    // Feature scaling parameters
    Eigen::VectorXd feature_means_;
    Eigen::VectorXd feature_stds_;
    double price_mean_;
    double price_std_;
    
    std::mt19937 rng_;
    
    // Helper methods
    Eigen::MatrixXd extract_features(const std::vector<MarketData>& data);
    Eigen::VectorXd calculate_technical_indicators(const std::vector<MarketData>& data, size_t index);
    void normalize_features(Eigen::MatrixXd& features);
    void fit_scaler(const Eigen::MatrixXd& features, const Eigen::VectorXd& prices);
    double calculate_sma(const std::vector<MarketData>& data, size_t index, int period);
    double calculate_ema(const std::vector<MarketData>& data, size_t index, int period);
    double calculate_rsi(const std::vector<MarketData>& data, size_t index, int period);
    double calculate_volatility(const std::vector<MarketData>& data, size_t index, int period);
};

} // namespace stock_predict