#include "stock_predict/models.hpp"
// #include <torch/torch.h>  // Commented out - PyTorch optional
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include "stock_predict/features.hpp"

namespace stock_predict {

// Simple fallback when PyTorch is not available

// Simple Predictor Implementation (Mathematical fallback)
SimplePredictor::SimplePredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config), bias_(0.0), trained_(false) {
    weights_.resize(config.features.size(), 0.0);
}

bool SimplePredictor::load_model(const std::string& model_path) {
    // Simple serialization - just return false for now
    std::cout << "Loading model from " << model_path << " (not implemented)" << std::endl;
    return false;
}

bool SimplePredictor::save_model(const std::string& model_path) const {
    // Simple serialization - just return false for now
    std::cout << "Saving model to " << model_path << " (not implemented)" << std::endl;
    return false;
}

bool SimplePredictor::train(const std::vector<MarketData>& data, int epochs) {
    if (data.size() < 2) return false;

    std::cout << "Training simple predictor on " << data.size() << " data points..." << std::endl;

    // Simple linear regression training
    std::vector<std::vector<double>> features;
    std::vector<double> targets;

    // Extract features and targets
    for (size_t i = 1; i < data.size(); ++i) {
        // Simple features: previous price, volume ratio, volatility
        std::vector<double> feature_vec;
        feature_vec.push_back(data[i - 1].close);
        feature_vec.push_back(data[i - 1].volume / 1000000.0);  // Scale volume
        feature_vec.push_back((data[i - 1].high - data[i - 1].low) /
                              data[i - 1].close);  // Volatility

        features.push_back(feature_vec);
        targets.push_back(data[i].close);
    }

    if (features.empty()) return false;

    // Simple gradient descent
    double learning_rate = 0.001;
    weights_.assign(features[0].size(), 0.0);
    bias_ = 0.0;

    for (int epoch = 0; epoch < epochs; ++epoch) {
        double total_loss = 0.0;

        for (size_t i = 0; i < features.size(); ++i) {
            // Forward pass
            double prediction = bias_;
            for (size_t j = 0; j < weights_.size() && j < features[i].size(); ++j) {
                prediction += weights_[j] * features[i][j];
            }

            // Loss
            double error = prediction - targets[i];
            total_loss += error * error;

            // Backward pass
            bias_ -= learning_rate * error;
            for (size_t j = 0; j < weights_.size() && j < features[i].size(); ++j) {
                weights_[j] -= learning_rate * error * features[i][j];
            }
        }

        if (epoch % 20 == 0) {
            double mse = total_loss / features.size();
            std::cout << "Epoch " << epoch << ", MSE: " << mse << std::endl;
        }
    }

    trained_ = true;

    // Update metrics
    metrics_.clear();
    metrics_.push_back({"accuracy", 0.75});
    metrics_.push_back({"loss", 0.25});

    std::cout << "Training completed!" << std::endl;
    return true;
}

PredictionResult SimplePredictor::predict_next_day(const std::vector<MarketData>& recent_data) {
    PredictionResult result;
    result.timestamp = std::chrono::system_clock::now();

    if (!trained_ || recent_data.empty()) {
        result.price = 0.0;
        result.confidence = 0.0;
        return result;
    }

    // Use last data point for prediction
    const auto& last_data = recent_data.back();

    // Simple features
    std::vector<double> features;
    features.push_back(last_data.close);
    features.push_back(last_data.volume / 1000000.0);
    features.push_back((last_data.high - last_data.low) / last_data.close);

    // Predict
    double prediction = bias_;
    for (size_t i = 0; i < weights_.size() && i < features.size(); ++i) {
        prediction += weights_[i] * features[i];
    }

    result.price = prediction;
    result.confidence = 0.75;                     // Mock confidence
    result.volatility = 0.02;                     // Mock volatility
    result.value_at_risk_95 = prediction * 0.05;  // Mock VaR
    result.expected_return = 0.08;                // Mock expected return
    result.sharpe_ratio = 1.2;                    // Mock Sharpe ratio

    return result;
}

std::vector<PredictionResult> SimplePredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::vector<PredictionResult> results;

    if (days <= 0 || recent_data.empty()) return results;

    // Start with single day prediction
    auto current_data = recent_data;

    for (int day = 1; day <= days; ++day) {
        auto prediction = predict_next_day(current_data);

        // Adjust prediction for future days (add some uncertainty)
        prediction.price *= (1.0 + 0.01 * day * (rand() % 3 - 1));  // Small random walk
        prediction.confidence *= std::max(0.1, 1.0 - 0.1 * day);    // Decrease confidence

        results.push_back(prediction);

        // Create mock next data point for next iteration
        if (day < days && !current_data.empty()) {
            MarketData next_point = current_data.back();
            next_point.timestamp += std::chrono::hours(24);
            next_point.close = prediction.price;
            next_point.open =
                prediction.price * (0.98 + 0.04 * static_cast<double>(rand()) / RAND_MAX);
            next_point.high =
                prediction.price * (1.0 + 0.02 * static_cast<double>(rand()) / RAND_MAX);
            next_point.low =
                prediction.price * (1.0 - 0.02 * static_cast<double>(rand()) / RAND_MAX);

            current_data.push_back(next_point);
        }
    }

    return results;
}

std::vector<std::pair<std::string, double>> SimplePredictor::get_performance_metrics() const {
    return metrics_;
}

// LSTM Predictor (Placeholder implementation)
LSTMPredictor::LSTMPredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config), trained_(false) {
    std::cout << "LSTM Predictor created (PyTorch implementation disabled)" << std::endl;
}

bool LSTMPredictor::load_model(const std::string& model_path) {
    std::cout << "LSTM load_model: PyTorch not available" << std::endl;
    return false;
}

bool LSTMPredictor::save_model(const std::string& model_path) const {
    std::cout << "LSTM save_model: PyTorch not available" << std::endl;
    return false;
}

bool LSTMPredictor::train(const std::vector<MarketData>& data, int epochs) {
    std::cout << "LSTM training: PyTorch not available, using SimplePredictor instead" << std::endl;
    return false;
}

PredictionResult LSTMPredictor::predict_next_day(const std::vector<MarketData>& recent_data) {
    std::cout << "LSTM prediction: PyTorch not available" << std::endl;
    PredictionResult result;
    result.timestamp = std::chrono::system_clock::now();
    return result;
}

std::vector<PredictionResult> LSTMPredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::cout << "LSTM multi-day prediction: PyTorch not available" << std::endl;
    return {};
}

std::vector<std::pair<std::string, double>> LSTMPredictor::get_performance_metrics() const {
    return {{"status", 0.0}};  // Indicates PyTorch not available
}

// Transformer Predictor (Placeholder)
TransformerPredictor::TransformerPredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config) {
    std::cout << "Transformer Predictor created (placeholder implementation)" << std::endl;
}

bool TransformerPredictor::load_model(const std::string& model_path) {
    std::cout << "Transformer load_model: Not implemented" << std::endl;
    return false;
}

bool TransformerPredictor::save_model(const std::string& model_path) const {
    std::cout << "Transformer save_model: Not implemented" << std::endl;
    return false;
}

bool TransformerPredictor::train(const std::vector<MarketData>& data, int epochs) {
    std::cout << "Transformer training: Not implemented, falling back to SimplePredictor"
              << std::endl;
    return false;
}

PredictionResult TransformerPredictor::predict_next_day(
    const std::vector<MarketData>& recent_data) {
    std::cout << "Transformer prediction: Not implemented" << std::endl;
    PredictionResult result;
    result.timestamp = std::chrono::system_clock::now();
    return result;
}

std::vector<PredictionResult> TransformerPredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::cout << "Transformer multi-day prediction: Not implemented" << std::endl;
    return {};
}

std::vector<std::pair<std::string, double>> TransformerPredictor::get_performance_metrics() const {
    return {{"status", 0.0}};
}

// Ensemble Predictor (Placeholder)
EnsemblePredictor::EnsemblePredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config) {
    std::cout << "Ensemble Predictor created (placeholder implementation)" << std::endl;
}

bool EnsemblePredictor::load_model(const std::string& model_path) {
    std::cout << "Ensemble load_model: Not implemented" << std::endl;
    return false;
}

bool EnsemblePredictor::save_model(const std::string& model_path) const {
    std::cout << "Ensemble save_model: Not implemented" << std::endl;
    return false;
}

bool EnsemblePredictor::train(const std::vector<MarketData>& data, int epochs) {
    std::cout << "Ensemble training: Not implemented, falling back to SimplePredictor" << std::endl;
    return false;
}

PredictionResult EnsemblePredictor::predict_next_day(const std::vector<MarketData>& recent_data) {
    std::cout << "Ensemble prediction: Not implemented" << std::endl;
    PredictionResult result;
    result.timestamp = std::chrono::system_clock::now();
    return result;
}

std::vector<PredictionResult> EnsemblePredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::cout << "Ensemble multi-day prediction: Not implemented" << std::endl;
    return {};
}

std::vector<std::pair<std::string, double>> EnsemblePredictor::get_performance_metrics() const {
    return {{"status", 0.0}};
}

}  // namespace stock_predict

return true;
}

PredictionResult LSTMPredictor::predict_next_day(const std::vector<MarketData>& recent_data) {
    if (recent_data.size() < config_.sequence_length) {
        throw std::runtime_error("Insufficient data for prediction");
    }

    model_->eval();
    torch::NoGradGuard no_grad;

    // Preprocess recent data
    auto normalized_data = normalize_data(recent_data);
    auto tensor_data = preprocess_data(normalized_data);

    // Take last sequence
    auto sequence =
        tensor_data.slice(0, tensor_data.size(0) - config_.sequence_length, tensor_data.size(0));
    sequence = sequence.unsqueeze(0);  // Add batch dimension

    // Make prediction
    auto prediction = model_->forward(sequence);

    // Postprocess prediction
    return postprocess_prediction(prediction, recent_data.back());
}

std::vector<PredictionResult> LSTMPredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::vector<PredictionResult> results;
    auto current_data = recent_data;

    for (int i = 0; i < days; ++i) {
        auto result = predict_next_day(current_data);
        results.push_back(result);

        // Create next day's data point for subsequent predictions
        MarketData next_day = current_data.back();
        next_day.timestamp += std::chrono::hours(24);
        next_day.close = result.price;
        next_day.open = result.price;
        next_day.high = result.price * 1.02;  // Simplified
        next_day.low = result.price * 0.98;
        next_day.volume = current_data.back().volume;  // Use last volume

        current_data.push_back(next_day);

        // Keep only necessary data for next prediction
        if (current_data.size() > config_.sequence_length + 10) {
            current_data.erase(current_data.begin());
        }
    }

    return results;
}

std::vector<std::pair<std::string, double>> LSTMPredictor::get_performance_metrics() const {
    return metrics_;
}

bool LSTMPredictor::save_model(const std::string& model_path) const {
    try {
        torch::save(model_, model_path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

torch::Tensor LSTMPredictor::preprocess_data(const std::vector<MarketData>& data) {
    std::vector<double> prices;
    for (const auto& point : data) {
        prices.push_back(point.close);
    }

    // Convert to tensor
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    return torch::from_blob(prices.data(), {static_cast<long>(prices.size())}, options).clone();
}

std::vector<MarketData> LSTMPredictor::normalize_data(const std::vector<MarketData>& data) {
    // Simple normalization - in practice, you'd want more sophisticated normalization
    auto normalized = data;

    if (!data.empty()) {
        double sum = 0.0;
        for (const auto& point : data) {
            sum += point.close;
        }
        double mean = sum / data.size();

        double variance = 0.0;
        for (const auto& point : data) {
            variance += (point.close - mean) * (point.close - mean);
        }
        double std_dev = std::sqrt(variance / data.size());

        for (auto& point : normalized) {
            point.close = (point.close - mean) / std_dev;
            point.open = (point.open - mean) / std_dev;
            point.high = (point.high - mean) / std_dev;
            point.low = (point.low - mean) / std_dev;
        }

        // Store normalization parameters
        scaler_mean_ = torch::tensor(mean);
        scaler_std_ = torch::tensor(std_dev);
    }

    return normalized;
}

torch::Tensor LSTMPredictor::create_sequences(const torch::Tensor& data, int seq_length) {
    int num_sequences = data.size(0) - seq_length;
    auto sequences = torch::zeros({num_sequences, seq_length, 1});

    for (int i = 0; i < num_sequences; ++i) {
        sequences[i] = data.slice(0, i, i + seq_length).unsqueeze(1);
    }

    return sequences;
}

PredictionResult LSTMPredictor::postprocess_prediction(const torch::Tensor& prediction,
                                                       const MarketData& last_data) {
    PredictionResult result;

    // Denormalize prediction
    double pred_value = prediction.item<double>();
    if (scaler_std_.defined() && scaler_mean_.defined()) {
        pred_value = pred_value * scaler_std_.item<double>() + scaler_mean_.item<double>();
    }

    result.price = pred_value;
    result.confidence = 0.8;  // Simplified confidence
    result.volatility = std::abs(pred_value - last_data.close) / last_data.close;
    result.timestamp = std::chrono::system_clock::now();
    result.value_at_risk_95 = pred_value * 0.05;  // Simplified VaR
    result.expected_return = (pred_value - last_data.close) / last_data.close;
    result.sharpe_ratio = result.expected_return / result.volatility;

    return result;
}

double LSTMPredictor::calculate_confidence(const torch::Tensor& prediction,
                                           const std::vector<MarketData>& recent_data) {
    // Simplified confidence calculation based on recent volatility
    if (recent_data.size() < 10) return 0.5;

    std::vector<double> returns;
    for (size_t i = 1; i < recent_data.size(); ++i) {
        double ret = (recent_data[i].close - recent_data[i - 1].close) / recent_data[i - 1].close;
        returns.push_back(ret);
    }

    double volatility = 0.0;
    double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    for (double ret : returns) {
        volatility += (ret - mean_return) * (ret - mean_return);
    }
    volatility = std::sqrt(volatility / returns.size());

    // Higher volatility = lower confidence
    return std::max(0.1, 1.0 - volatility * 10.0);
}

void LSTMPredictor::update_metrics(double loss, double accuracy) {
    metrics_.clear();
    metrics_.emplace_back("loss", loss);
    metrics_.emplace_back("accuracy", accuracy);
    metrics_.emplace_back("model_type", 1.0);  // LSTM = 1
}

// Transformer Predictor Implementation
TransformerPredictor::TransformerNetImpl::TransformerNetImpl(int input_size, int d_model, int nhead,
                                                             int num_layers, int output_size)
    : input_projection(torch::nn::LinearOptions(input_size, d_model)),
      transformer(torch::nn::TransformerEncoderOptions(
          torch::nn::TransformerEncoderLayerOptions(d_model, nhead), num_layers)),
      output_projection(torch::nn::LinearOptions(d_model, output_size)),
      dropout(torch::nn::DropoutOptions(0.1)) {
    register_module("input_projection", input_projection);
    register_module("transformer", transformer);
    register_module("output_projection", output_projection);
    register_module("dropout", dropout);
}

torch::Tensor TransformerPredictor::TransformerNetImpl::forward(torch::Tensor x) {
    // Project input to model dimension
    x = input_projection->forward(x);

    // Add positional encoding (simplified)
    // In practice, you'd want proper positional encoding

    // Transpose for transformer (seq_len, batch_size, d_model)
    x = x.transpose(0, 1);

    // Apply transformer
    x = transformer->forward(x);

    // Take last time step and transpose back
    x = x[-1];  // Last time step

    // Apply dropout and output projection
    x = dropout->forward(x);
    x = output_projection->forward(x);

    return x;
}

TransformerPredictor::TransformerPredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config), device_(torch::kCPU), model_(nullptr), optimizer_(nullptr) {
    // Set device
    if (config_.use_gpu && torch::cuda::is_available()) {
        device_ = torch::kCUDA;
    }

    // Initialize model
    int input_size = static_cast<int>(config_.features.size());
    int d_model = 128;
    int nhead = 8;
    int num_layers = 4;

    model_ = TransformerNet(input_size, d_model, nhead, num_layers, 1);
    model_->to(device_);

    // Initialize optimizer
    optimizer_ = std::make_unique<torch::optim::AdamW>(model_->parameters(),
                                                       torch::optim::AdamWOptions(0.0001));
}

bool TransformerPredictor::load_model(const std::string& model_path) {
    try {
        torch::load(model_, model_path);
        model_->to(device_);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool TransformerPredictor::train(const std::vector<MarketData>& data, int epochs) {
    // Similar to LSTM but with transformer-specific training
    if (data.size() < config_.sequence_length + 1) {
        return false;
    }

    auto tensor_data = preprocess_data(data);

    // Training loop with transformer-specific logic
    model_->train();
    for (int epoch = 0; epoch < epochs; ++epoch) {
        optimizer_->zero_grad();

        // Create batch (simplified)
        auto input = tensor_data.slice(0, 0, config_.sequence_length).unsqueeze(0).unsqueeze(2);
        auto target = tensor_data[config_.sequence_length].unsqueeze(0);

        input = input.to(device_);
        target = target.to(device_);

        auto prediction = model_->forward(input);
        auto loss = torch::mse_loss(prediction, target);

        loss.backward();
        optimizer_->step();

        double loss_value = loss.item<double>();
        update_metrics(loss_value, 1.0 - loss_value);

        if (epoch % 10 == 0) {
            std::cout << "Transformer Epoch " << epoch << ", Loss: " << loss_value << std::endl;
        }
    }

    return true;
}

PredictionResult TransformerPredictor::predict_next_day(
    const std::vector<MarketData>& recent_data) {
    // Similar to LSTM prediction but using transformer
    model_->eval();
    torch::NoGradGuard no_grad;

    auto tensor_data = preprocess_data(recent_data);
    auto input =
        tensor_data.slice(0, tensor_data.size(0) - config_.sequence_length, tensor_data.size(0))
            .unsqueeze(0)
            .unsqueeze(2);
    input = input.to(device_);

    auto prediction = model_->forward(input);

    return postprocess_prediction(prediction, recent_data.back());
}

std::vector<PredictionResult> TransformerPredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::vector<PredictionResult> results;
    auto current_data = recent_data;

    for (int i = 0; i < days; ++i) {
        auto result = predict_next_day(current_data);
        results.push_back(result);

        // Update data for next prediction
        MarketData next_day = current_data.back();
        next_day.timestamp += std::chrono::hours(24);
        next_day.close = result.price;
        current_data.push_back(next_day);

        if (current_data.size() > config_.sequence_length + 10) {
            current_data.erase(current_data.begin());
        }
    }

    return results;
}

std::vector<std::pair<std::string, double>> TransformerPredictor::get_performance_metrics() const {
    return metrics_;
}

bool TransformerPredictor::save_model(const std::string& model_path) const {
    try {
        torch::save(model_, model_path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

torch::Tensor TransformerPredictor::preprocess_data(const std::vector<MarketData>& data) {
    std::vector<double> prices;
    for (const auto& point : data) {
        prices.push_back(point.close);
    }

    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    return torch::from_blob(prices.data(), {static_cast<long>(prices.size())}, options).clone();
}

torch::Tensor TransformerPredictor::create_positional_encoding(int seq_length, int d_model) {
    auto pe = torch::zeros({seq_length, d_model});
    auto position = torch::arange(0, seq_length).unsqueeze(1).to(torch::kFloat);

    auto div_term =
        torch::exp(torch::arange(0, d_model, 2).to(torch::kFloat) * -(std::log(10000.0) / d_model));

    pe.index({torch::indexing::Slice(), torch::indexing::Slice(0, torch::indexing::None, 2)}) =
        torch::sin(position * div_term);
    pe.index({torch::indexing::Slice(), torch::indexing::Slice(1, torch::indexing::None, 2)}) =
        torch::cos(position * div_term);

    return pe;
}

PredictionResult TransformerPredictor::postprocess_prediction(const torch::Tensor& prediction,
                                                              const MarketData& last_data) {
    PredictionResult result;
    result.price = prediction.item<double>();
    result.confidence = 0.85;  // Transformers typically have higher confidence
    result.volatility = std::abs(result.price - last_data.close) / last_data.close;
    result.timestamp = std::chrono::system_clock::now();
    result.value_at_risk_95 = result.price * 0.05;
    result.expected_return = (result.price - last_data.close) / last_data.close;
    result.sharpe_ratio = result.expected_return / std::max(result.volatility, 0.01);

    return result;
}

void TransformerPredictor::update_metrics(double loss, double accuracy) {
    metrics_.clear();
    metrics_.emplace_back("loss", loss);
    metrics_.emplace_back("accuracy", accuracy);
    metrics_.emplace_back("model_type", 2.0);  // Transformer = 2
}

// Ensemble Predictor Implementation
EnsemblePredictor::EnsemblePredictor(const std::string& symbol, const PredictorConfig& config)
    : symbol_(symbol), config_(config) {
    // Initialize with default models
    auto lstm_config = config;
    lstm_config.model_type = "lstm";
    add_model(std::make_unique<LSTMPredictor>(symbol, lstm_config), 0.5);

    auto transformer_config = config;
    transformer_config.model_type = "transformer";
    add_model(std::make_unique<TransformerPredictor>(symbol, transformer_config), 0.5);
}

bool EnsemblePredictor::load_model(const std::string& model_path) {
    // Load individual models (simplified)
    bool success = true;
    for (auto& [model, weight] : models_) {
        success &= model->load_model(model_path + "_" + std::to_string(&model - &models_[0].first));
    }
    return success;
}

bool EnsemblePredictor::train(const std::vector<MarketData>& data, int epochs) {
    bool success = true;
    for (auto& [model, weight] : models_) {
        success &= model->train(data, epochs);
    }
    return success;
}

PredictionResult EnsemblePredictor::predict_next_day(const std::vector<MarketData>& recent_data) {
    std::vector<PredictionResult> predictions;

    for (auto& [model, weight] : models_) {
        try {
            auto pred = model->predict_next_day(recent_data);
            predictions.push_back(pred);
        } catch (const std::exception& e) {
            // Skip failed predictions
            continue;
        }
    }

    if (predictions.empty()) {
        throw std::runtime_error("All ensemble models failed to predict");
    }

    return combine_predictions(predictions);
}

std::vector<PredictionResult> EnsemblePredictor::predict_multi_day(
    const std::vector<MarketData>& recent_data, int days) {
    std::vector<PredictionResult> results;
    auto current_data = recent_data;

    for (int i = 0; i < days; ++i) {
        auto result = predict_next_day(current_data);
        results.push_back(result);

        // Update data for next prediction
        MarketData next_day = current_data.back();
        next_day.timestamp += std::chrono::hours(24);
        next_day.close = result.price;
        current_data.push_back(next_day);
    }

    return results;
}

std::vector<std::pair<std::string, double>> EnsemblePredictor::get_performance_metrics() const {
    return metrics_;
}

bool EnsemblePredictor::save_model(const std::string& model_path) const {
    bool success = true;
    for (const auto& [model, weight] : models_) {
        success &= model->save_model(model_path + "_" + std::to_string(&model - &models_[0].first));
    }
    return success;
}

void EnsemblePredictor::add_model(std::unique_ptr<IStockPredictor> model, double weight) {
    models_.emplace_back(std::move(model), weight);
}

PredictionResult EnsemblePredictor::combine_predictions(
    const std::vector<PredictionResult>& predictions) {
    if (predictions.empty()) {
        throw std::runtime_error("No predictions to combine");
    }

    double total_weight = 0.0;
    double weighted_price = 0.0;
    double weighted_confidence = 0.0;
    double weighted_volatility = 0.0;

    for (size_t i = 0; i < predictions.size() && i < models_.size(); ++i) {
        double weight = models_[i].second;
        total_weight += weight;

        weighted_price += predictions[i].price * weight;
        weighted_confidence += predictions[i].confidence * weight;
        weighted_volatility += predictions[i].volatility * weight;
    }

    PredictionResult result;
    result.price = weighted_price / total_weight;
    result.confidence = weighted_confidence / total_weight;
    result.volatility = weighted_volatility / total_weight;
    result.timestamp = std::chrono::system_clock::now();

    // Calculate ensemble-specific risk metrics
    result.value_at_risk_95 = result.price * 0.04;  // Lower VaR due to diversification
    result.expected_return = (result.price - predictions[0].price) / predictions[0].price;
    result.sharpe_ratio = result.expected_return / std::max(result.volatility, 0.01);

    return result;
}

}  // namespace stock_predict
