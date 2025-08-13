#pragma once

#include <torch/torch.h>
#include <memory>
#include "stock_predictor.hpp"

namespace stock_predict {

/**
 * @brief LSTM-based stock price predictor using PyTorch C++
 */
class LSTMPredictor : public IStockPredictor {
   public:
    explicit LSTMPredictor(const std::string& symbol, const PredictorConfig& config = {});
    ~LSTMPredictor() override = default;

    // IStockPredictor interface
    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<MarketData>& data, int epochs = 100) override;
    PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) override;
    std::vector<PredictionResult> predict_multi_day(const std::vector<MarketData>& recent_data,
                                                    int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

   private:
    struct LSTMNetImpl : torch::nn::Module {
        LSTMNetImpl(int input_size, int hidden_size, int num_layers, int output_size);
        torch::Tensor forward(torch::Tensor x);

        torch::nn::LSTM lstm{nullptr};
        torch::nn::Linear fc1{nullptr};
        torch::nn::Dropout dropout{nullptr};
        torch::nn::Linear fc2{nullptr};
    };
    TORCH_MODULE(LSTMNet);

    // Configuration and state
    std::string symbol_;
    PredictorConfig config_;
    torch::Device device_;
    LSTMNet model_;
    torch::optim::Adam optimizer_{nullptr};

    // Data preprocessing
    torch::Tensor scaler_mean_;
    torch::Tensor scaler_std_;

    // Performance tracking
    mutable std::vector<std::pair<std::string, double>> metrics_;

    // Helper methods
    torch::Tensor preprocess_data(const std::vector<MarketData>& data);
    std::vector<MarketData> normalize_data(const std::vector<MarketData>& data);
    torch::Tensor create_sequences(const torch::Tensor& data, int seq_length);
    PredictionResult postprocess_prediction(const torch::Tensor& prediction,
                                            const MarketData& last_data);
    double calculate_confidence(const torch::Tensor& prediction,
                                const std::vector<MarketData>& recent_data);
    void update_metrics(double loss, double accuracy);
};

/**
 * @brief Transformer-based stock price predictor
 */
class TransformerPredictor : public IStockPredictor {
   public:
    explicit TransformerPredictor(const std::string& symbol, const PredictorConfig& config = {});
    ~TransformerPredictor() override = default;

    // IStockPredictor interface
    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<MarketData>& data, int epochs = 100) override;
    PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) override;
    std::vector<PredictionResult> predict_multi_day(const std::vector<MarketData>& recent_data,
                                                    int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

   private:
    struct TransformerNetImpl : torch::nn::Module {
        TransformerNetImpl(int input_size, int d_model, int nhead, int num_layers, int output_size);
        torch::Tensor forward(torch::Tensor x);

        torch::nn::Linear input_projection{nullptr};
        torch::nn::TransformerEncoder transformer{nullptr};
        torch::nn::Linear output_projection{nullptr};
        torch::nn::Dropout dropout{nullptr};
    };
    TORCH_MODULE(TransformerNet);

    // Configuration and state
    std::string symbol_;
    PredictorConfig config_;
    torch::Device device_;
    TransformerNet model_;
    torch::optim::AdamW optimizer_{nullptr};

    // Data preprocessing
    torch::Tensor scaler_mean_;
    torch::Tensor scaler_std_;

    // Performance tracking
    mutable std::vector<std::pair<std::string, double>> metrics_;

    // Helper methods
    torch::Tensor preprocess_data(const std::vector<MarketData>& data);
    torch::Tensor create_positional_encoding(int seq_length, int d_model);
    PredictionResult postprocess_prediction(const torch::Tensor& prediction,
                                            const MarketData& last_data);
    void update_metrics(double loss, double accuracy);
};

/**
 * @brief Ensemble predictor combining multiple models
 */
class EnsemblePredictor : public IStockPredictor {
   public:
    explicit EnsemblePredictor(const std::string& symbol, const PredictorConfig& config = {});
    ~EnsemblePredictor() override = default;

    // IStockPredictor interface
    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<MarketData>& data, int epochs = 100) override;
    PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) override;
    std::vector<PredictionResult> predict_multi_day(const std::vector<MarketData>& recent_data,
                                                    int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

    // Add models to ensemble
    void add_model(std::unique_ptr<IStockPredictor> model, double weight = 1.0);

   private:
    std::string symbol_;
    PredictorConfig config_;
    std::vector<std::pair<std::unique_ptr<IStockPredictor>, double>> models_;
    mutable std::vector<std::pair<std::string, double>> metrics_;

    // Ensemble methods
    PredictionResult combine_predictions(const std::vector<PredictionResult>& predictions);
};

}  // namespace stock_predict
