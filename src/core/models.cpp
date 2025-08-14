#include "stock_predict/models.hpp"
#include "stock_predict/features.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
#include <random>
#include <chrono>

namespace stock_predict {

// ===================== SimplePredictor Implementation =====================

SimplePredictor::SimplePredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config), is_trained_(false), training_accuracy_(0.0), rng_(std::random_device{}()) {
}

bool SimplePredictor::load_model(const std::string& model_path) {
    try {
        std::ifstream file(model_path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open file for reading: " << model_path << std::endl;
            return false;
        }
        
        // Load basic info
        size_t symbol_size;
        file.read(reinterpret_cast<char*>(&symbol_size), sizeof(symbol_size));
        symbol_.resize(symbol_size);
        file.read(&symbol_[0], symbol_size);
        file.read(reinterpret_cast<char*>(&training_accuracy_), sizeof(training_accuracy_));
        file.read(reinterpret_cast<char*>(&price_mean_), sizeof(price_mean_));
        file.read(reinterpret_cast<char*>(&price_std_), sizeof(price_std_));
        
        // Load weights
        int weights_size;
        file.read(reinterpret_cast<char*>(&weights_size), sizeof(weights_size));
        weights_.resize(weights_size);
        file.read(reinterpret_cast<char*>(weights_.data()), weights_size * sizeof(double));
        file.read(reinterpret_cast<char*>(&bias_), sizeof(bias_));
        
        // Load feature scaling parameters
        int feature_count;
        file.read(reinterpret_cast<char*>(&feature_count), sizeof(feature_count));
        feature_means_.resize(feature_count);
        feature_stds_.resize(feature_count);
        file.read(reinterpret_cast<char*>(feature_means_.data()), feature_count * sizeof(double));
        file.read(reinterpret_cast<char*>(feature_stds_.data()), feature_count * sizeof(double));
        
        file.close();
        is_trained_ = true;
        std::cout << "Model loaded successfully for " << symbol_ << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        return false;
    }
}

bool SimplePredictor::train(const std::vector<MarketData>& training_data, int epochs) {
    if (training_data.size() < static_cast<size_t>(config_.sequence_length + 1)) {
        std::cerr << "Not enough training data. Need at least " << config_.sequence_length + 1 << " samples." << std::endl;
        return false;
    }
    
    try {
        std::cout << "Training " << symbol_ << " with " << epochs << " epochs..." << std::endl;
        
        // Extract features and targets
        Eigen::MatrixXd features = extract_features(training_data);
        Eigen::VectorXd targets(features.cols());
        
        // Extract target prices
        for (Eigen::Index i = 0; i < features.cols(); ++i) {
            targets(i) = training_data[i + config_.sequence_length].close;
        }
        
        // Fit feature scaler
        fit_scaler(features, targets);
        
        // Normalize features
        normalize_features(features);
        
        // Normalize targets
        Eigen::VectorXd normalized_targets = (targets.array() - price_mean_) / price_std_;
        
        // Simple linear regression using normal equation: w = (X^T X)^(-1) X^T y
        Eigen::MatrixXd X = features.transpose();
        
        // Add regularization to prevent overfitting
        double lambda = 0.01;
        Eigen::MatrixXd XtX = X.transpose() * X + lambda * Eigen::MatrixXd::Identity(X.cols(), X.cols());
        Eigen::MatrixXd XtX_inv = XtX.inverse();
        weights_ = XtX_inv * X.transpose() * normalized_targets;
        bias_ = normalized_targets.mean();
        
        // Calculate training accuracy (R-squared)
        Eigen::VectorXd predictions = X * weights_;
        for (int i = 0; i < predictions.size(); ++i) {
            predictions(i) += bias_;
        }
        
        double ss_res = (normalized_targets - predictions).array().square().sum();
        double ss_tot = (normalized_targets.array() - normalized_targets.mean()).square().sum();
        training_accuracy_ = 1.0 - (ss_res / ss_tot);
        
        is_trained_ = true;
        
        std::cout << "Training completed for " << symbol_ 
                  << ". R-squared: " << training_accuracy_ << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Training failed: " << e.what() << std::endl;
        return false;
    }
}

PredictionResult SimplePredictor::predict_next_day(const std::vector<MarketData>& recent_data) {
    PredictionResult result{};
    
    if (!is_trained_) {
        std::cerr << "Model not trained. Cannot make predictions." << std::endl;
        return result;
    }
    
    if (recent_data.size() < static_cast<size_t>(config_.sequence_length)) {
        std::cerr << "Not enough recent data for prediction. Need " << config_.sequence_length << " samples." << std::endl;
        return result;
    }
    
    try {
        // Use the last sequence_length samples
        std::vector<MarketData> input_data(recent_data.end() - config_.sequence_length, recent_data.end());
        
        // Extract features for the most recent sequence
        Eigen::VectorXd features = calculate_technical_indicators(input_data, input_data.size() - 1);
        
        // Normalize features
        for (int i = 0; i < features.size() && i < feature_means_.size(); ++i) {
            if (feature_stds_(i) > 1e-8) {
                features(i) = (features(i) - feature_means_(i)) / feature_stds_(i);
            }
        }
        
        // Make prediction
        double normalized_prediction = weights_.dot(features) + bias_;
        double predicted_price = normalized_prediction * price_std_ + price_mean_;
        
        // Calculate confidence based on recent volatility
        double recent_volatility = calculate_volatility(input_data, input_data.size() - 1, 10);
        double confidence = std::max(0.1, std::min(0.9, 1.0 - recent_volatility));
        
        // Fill result
        result.price = std::max(0.01, predicted_price);  // Ensure positive price
        result.confidence = confidence;
        result.volatility = recent_volatility;
        result.timestamp = std::chrono::system_clock::now();
        
        // Simple risk metrics
        double expected_return = (predicted_price - recent_data.back().close) / recent_data.back().close;
        result.expected_return = expected_return;
        result.value_at_risk_95 = recent_data.back().close * recent_volatility * 1.96;  // 95% VaR
        result.sharpe_ratio = expected_return / (recent_volatility + 1e-8);  // Avoid division by zero
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Prediction failed: " << e.what() << std::endl;
        return result;
    }
}

std::vector<PredictionResult> SimplePredictor::predict_multi_day(const std::vector<MarketData>& recent_data, int days) {
    std::vector<PredictionResult> results;
    
    if (!is_trained_ || days <= 0) {
        return results;
    }
    
    // For multi-day prediction, we'll predict each day iteratively
    std::vector<MarketData> current_data = recent_data;
    
    for (int day = 0; day < days; ++day) {
        PredictionResult prediction = predict_next_day(current_data);
        if (prediction.price <= 0) {
            break;  // Stop if prediction fails
        }
        
        results.push_back(prediction);
        
        // Create synthetic data point for next prediction
        if (day < days - 1) {  // Don't create synthetic data for the last iteration
            MarketData synthetic_point;
            synthetic_point.symbol = symbol_;
            synthetic_point.timestamp = prediction.timestamp;
            synthetic_point.close = prediction.price;
            synthetic_point.open = prediction.price * (1.0 + 0.001 * (rng_() % 11 - 5));  // Small random variation
            synthetic_point.high = std::max(synthetic_point.open, synthetic_point.close) * 1.02;
            synthetic_point.low = std::min(synthetic_point.open, synthetic_point.close) * 0.98;
            synthetic_point.volume = current_data.empty() ? 1000000 : current_data.back().volume;
            synthetic_point.adjusted_close = synthetic_point.close;
            
            current_data.push_back(synthetic_point);
        }
    }
    
    return results;
}

std::vector<std::pair<std::string, double>> SimplePredictor::get_performance_metrics() const {
    std::vector<std::pair<std::string, double>> metrics;
    
    if (is_trained_) {
        metrics.emplace_back("R-squared", training_accuracy_);
        metrics.emplace_back("Number of Features", static_cast<double>(weights_.size()));
        metrics.emplace_back("Model Complexity", static_cast<double>(weights_.norm()));
        metrics.emplace_back("Feature Mean Magnitude", feature_means_.norm());
        metrics.emplace_back("Feature Std Magnitude", feature_stds_.norm());
    }
    
    return metrics;
}

bool SimplePredictor::save_model(const std::string& model_path) const {
    if (!is_trained_) {
        std::cerr << "Cannot save untrained model." << std::endl;
        return false;
    }
    
    try {
        std::ofstream file(model_path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Cannot open file for writing: " << model_path << std::endl;
            return false;
        }
        
        // Save basic info
        size_t symbol_size = symbol_.size();
        file.write(reinterpret_cast<const char*>(&symbol_size), sizeof(symbol_size));
        file.write(symbol_.c_str(), symbol_size);
        file.write(reinterpret_cast<const char*>(&training_accuracy_), sizeof(training_accuracy_));
        file.write(reinterpret_cast<const char*>(&price_mean_), sizeof(price_mean_));
        file.write(reinterpret_cast<const char*>(&price_std_), sizeof(price_std_));
        
        // Save weights
        int weights_size = weights_.size();
        file.write(reinterpret_cast<const char*>(&weights_size), sizeof(weights_size));
        file.write(reinterpret_cast<const char*>(weights_.data()), weights_size * sizeof(double));
        file.write(reinterpret_cast<const char*>(&bias_), sizeof(bias_));
        
        // Save feature scaling parameters
        int feature_count = feature_means_.size();
        file.write(reinterpret_cast<const char*>(&feature_count), sizeof(feature_count));
        file.write(reinterpret_cast<const char*>(feature_means_.data()), feature_count * sizeof(double));
        file.write(reinterpret_cast<const char*>(feature_stds_.data()), feature_count * sizeof(double));
        
        file.close();
        std::cout << "Model saved successfully for " << symbol_ << " to " << model_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save model: " << e.what() << std::endl;
        return false;
    }
}

Eigen::MatrixXd SimplePredictor::extract_features(const std::vector<MarketData>& data) {
    if (data.size() < static_cast<size_t>(config_.sequence_length)) {
        return Eigen::MatrixXd();
    }
    
    size_t num_samples = data.size() - config_.sequence_length + 1;
    const int num_features = 15;  // Number of technical indicators
    
    Eigen::MatrixXd features(num_features, num_samples);
    
    for (size_t i = 0; i < num_samples; ++i) {
        size_t data_index = i + config_.sequence_length - 1;
        Eigen::VectorXd sample_features = calculate_technical_indicators(data, data_index);
        features.col(i) = sample_features;
    }
    
    return features;
}

Eigen::VectorXd SimplePredictor::calculate_technical_indicators(const std::vector<MarketData>& data, size_t index) {
    Eigen::VectorXd features(15);
    
    if (index >= data.size()) {
        return features;
    }
    
    // Basic price features
    features(0) = data[index].close;
    features(1) = data[index].volume;
    features(2) = data[index].high - data[index].low;  // Daily range
    
    // Moving averages
    features(3) = calculate_sma(data, index, 5);
    features(4) = calculate_sma(data, index, 10);
    features(5) = calculate_sma(data, index, 20);
    features(6) = calculate_ema(data, index, 12);
    features(7) = calculate_ema(data, index, 26);
    
    // Technical indicators
    features(8) = calculate_rsi(data, index, 14);
    features(9) = calculate_volatility(data, index, 10);
    features(10) = calculate_volatility(data, index, 20);
    
    // Price ratios
    if (features(3) > 0) features(11) = data[index].close / features(3);  // Price/SMA5
    if (features(4) > 0) features(12) = data[index].close / features(4);  // Price/SMA10
    if (features(5) > 0) features(13) = data[index].close / features(5);  // Price/SMA20
    
    // Momentum
    if (index > 0) {
        features(14) = (data[index].close - data[index-1].close) / data[index-1].close;
    }
    
    return features;
}

void SimplePredictor::normalize_features(Eigen::MatrixXd& features) {
    for (int i = 0; i < features.rows(); ++i) {
        for (int j = 0; j < features.cols(); ++j) {
            if (feature_stds_(i) > 1e-8) {
                features(i, j) = (features(i, j) - feature_means_(i)) / feature_stds_(i);
            }
        }
    }
}

void SimplePredictor::fit_scaler(const Eigen::MatrixXd& features, const Eigen::VectorXd& prices) {
    // Calculate feature means and standard deviations
    feature_means_ = features.rowwise().mean();
    feature_stds_ = Eigen::VectorXd(features.rows());
    
    for (int i = 0; i < features.rows(); ++i) {
        double variance = (features.row(i).array() - feature_means_(i)).square().mean();
        feature_stds_(i) = std::sqrt(variance + 1e-8);  // Add small epsilon to avoid division by zero
    }
    
    // Calculate price statistics
    price_mean_ = prices.mean();
    double price_variance = (prices.array() - price_mean_).square().mean();
    price_std_ = std::sqrt(price_variance + 1e-8);  // Add small epsilon
}

double SimplePredictor::calculate_sma(const std::vector<MarketData>& data, size_t index, int period) {
    if (static_cast<int>(index) < period - 1) {
        period = index + 1;
    }
    
    double sum = 0.0;
    for (int i = 0; i < period; ++i) {
        sum += data[index - i].close;
    }
    
    return sum / period;
}

double SimplePredictor::calculate_ema(const std::vector<MarketData>& data, size_t index, int period) {
    if (index == 0) {
        return data[0].close;
    }
    
    double alpha = 2.0 / (period + 1);
    double ema = data[0].close;
    
    for (size_t i = 1; i <= index; ++i) {
        ema = alpha * data[i].close + (1 - alpha) * ema;
    }
    
    return ema;
}

double SimplePredictor::calculate_rsi(const std::vector<MarketData>& data, size_t index, int period) {
    if (static_cast<int>(index) < period) {
        return 50.0;  // Neutral RSI
    }
    
    double gain_sum = 0.0;
    double loss_sum = 0.0;
    
    for (int i = 1; i <= period; ++i) {
        double change = data[index - i + 1].close - data[index - i].close;
        if (change > 0) {
            gain_sum += change;
        } else {
            loss_sum += -change;
        }
    }
    
    double avg_gain = gain_sum / period;
    double avg_loss = loss_sum / period;
    
    if (avg_loss == 0) {
        return 100.0;
    }
    
    double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

double SimplePredictor::calculate_volatility(const std::vector<MarketData>& data, size_t index, int period) {
    if (static_cast<int>(index) < period - 1) {
        period = index + 1;
    }
    
    std::vector<double> returns;
    for (int i = 1; i < period; ++i) {
        size_t curr_idx = index - i + 1;
        size_t prev_idx = index - i;
        if (curr_idx < data.size() && prev_idx < data.size() && data[prev_idx].close > 0) {
            double ret = (data[curr_idx].close - data[prev_idx].close) / data[prev_idx].close;
            returns.push_back(ret);
        }
    }
    
    if (returns.empty()) {
        return 0.1;  // Default volatility
    }
    
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double variance = 0.0;
    
    for (double ret : returns) {
        variance += (ret - mean_return) * (ret - mean_return);
    }
    
    variance /= returns.size();
    return std::sqrt(variance);
}

} // namespace stock_predict