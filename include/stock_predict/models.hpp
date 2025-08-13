#pragma once

// #include <torch/torch.h>  // Comment out PyTorch for now
#include <memory>
#include "stock_predictor.hpp"

namespace stock_predict {

/**
 * @brief Transformer-based predictor (placeholder)
 */
class TransformerPredictor : public IStockPredictor {
   public:
    explicit TransformerPredictor(const std::string& symbol, const PredictorConfig& config = {});
    ~TransformerPredictor() override = default;

    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<MarketData>& data, int epochs = 100) override;
    PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) override;
    std::vector<PredictionResult> predict_multi_day(const std::vector<MarketData>& recent_data,
                                                    int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

   private:
    std::string symbol_;
    PredictorConfig config_;
};

/**
 * @brief Ensemble predictor (placeholder)
 */
class EnsemblePredictor : public IStockPredictor {
   public:
    explicit EnsemblePredictor(const std::string& symbol, const PredictorConfig& config = {});
    ~EnsemblePredictor() override = default;

    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<MarketData>& data, int epochs = 100) override;
    PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) override;
    std::vector<PredictionResult> predict_multi_day(const std::vector<MarketData>& recent_data,
                                                    int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

   private:
    std::string symbol_;
    PredictorConfig config_;
};

}  // namespace stock_predict

/**
 * @brief Simple mathematical predictor (fallback when PyTorch unavailable)
 */
class SimplePredictor : public stock_predict::IStockPredictor {
   public:
    explicit SimplePredictor(const std::string& symbol, const stock_predict::PredictorConfig& config = {});
    ~SimplePredictor() override = default;

    // IStockPredictor interface
    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<stock_predict::MarketData>& data, int epochs = 100) override;
    stock_predict::PredictionResult predict_next_day(const std::vector<stock_predict::MarketData>& recent_data) override;
    std::vector<stock_predict::PredictionResult> predict_multi_day(const std::vector<stock_predict::MarketData>& recent_data,
                                                    int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

   private:
    std::string symbol_;
    stock_predict::PredictorConfig config_;
    std::vector<double> weights_;
    double bias_;
    bool trained_;
    std::vector<std::pair<std::string, double>> metrics_;
};

/**
 * @brief LSTM-based stock price predictor (requires PyTorch)
 * Note: Currently disabled - requires PyTorch C++ installation
 */
class LSTMPredictor : public stock_predict::IStockPredictor {
   public:
    explicit LSTMPredictor(const std::string& symbol,
                           const stock_predict::PredictorConfig& config = {});
    ~LSTMPredictor() override = default;

    // IStockPredictor interface
    bool load_model(const std::string& model_path) override;
    bool train(const std::vector<stock_predict::MarketData>& data, int epochs = 100) override;
    stock_predict::PredictionResult predict_next_day(
        const std::vector<stock_predict::MarketData>& recent_data) override;
    std::vector<stock_predict::PredictionResult> predict_multi_day(
        const std::vector<stock_predict::MarketData>& recent_data, int days) override;
    std::vector<std::pair<std::string, double>> get_performance_metrics() const override;
    bool save_model(const std::string& model_path) const override;

   private:
    std::string symbol_;
    stock_predict::PredictorConfig config_;
    // PyTorch components would go here when available
    bool trained_;
};

// Helper methods (disabled - requires PyTorch)
// torch::Tensor preprocess_data(const std::vector<stock_predict::MarketData>& data);
// std::vector<stock_predict::MarketData> normalize_data(const
// std::vector<stock_predict::MarketData>& data); torch::Tensor create_sequences(const
// torch::Tensor& data, int seq_length); stock_predict::PredictionResult
// postprocess_prediction(const torch::Tensor& prediction,
//                                         const stock_predict::MarketData& last_data);
// double calculate_confidence(const torch::Tensor& prediction,
                            const std::vector<MarketData>& recent_data);
                            void update_metrics(double loss, double accuracy);
                            }
                            ;

                            /**
                             * @brief Transformer-based stock price predictor
                             */
                            class TransformerPredictor : public IStockPredictor {
                               public:
                                explicit TransformerPredictor(const std::string& symbol,
                                                              const PredictorConfig& config = {});
                                ~TransformerPredictor() override = default;

                                // IStockPredictor interface
                                bool load_model(const std::string& model_path) override;
                                bool train(const std::vector<MarketData>& data,
                                           int epochs = 100) override;
                                PredictionResult predict_next_day(
                                    const std::vector<MarketData>& recent_data) override;
                                std::vector<PredictionResult> predict_multi_day(
                                    const std::vector<MarketData>& recent_data, int days) override;
                                std::vector<std::pair<std::string, double>>
                                get_performance_metrics() const override;
                                bool save_model(const std::string& model_path) const override;

                               private:
                                struct TransformerNetImpl : torch::nn::Module {
                                    TransformerNetImpl(int input_size, int d_model, int nhead,
                                                       int num_layers, int output_size);
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
                                torch::Tensor create_positional_encoding(int seq_length,
                                                                         int d_model);
                                PredictionResult postprocess_prediction(
                                    const torch::Tensor& prediction, const MarketData& last_data);
                                void update_metrics(double loss, double accuracy);
                            };

                            /**
                             * @brief Ensemble predictor combining multiple models
                             */
                            class EnsemblePredictor : public IStockPredictor {
                               public:
                                explicit EnsemblePredictor(const std::string& symbol,
                                                           const PredictorConfig& config = {});
                                ~EnsemblePredictor() override = default;

                                // IStockPredictor interface
                                bool load_model(const std::string& model_path) override;
                                bool train(const std::vector<MarketData>& data,
                                           int epochs = 100) override;
                                PredictionResult predict_next_day(
                                    const std::vector<MarketData>& recent_data) override;
                                std::vector<PredictionResult> predict_multi_day(
                                    const std::vector<MarketData>& recent_data, int days) override;
                                std::vector<std::pair<std::string, double>>
                                get_performance_metrics() const override;
                                bool save_model(const std::string& model_path) const override;

                                // Add models to ensemble
                                void add_model(std::unique_ptr<IStockPredictor> model,
                                               double weight = 1.0);

                               private:
                                std::string symbol_;
                                PredictorConfig config_;
                                std::vector<std::pair<std::unique_ptr<IStockPredictor>, double>>
                                    models_;
                                mutable std::vector<std::pair<std::string, double>> metrics_;

                                // Ensemble methods
                                PredictionResult combine_predictions(
                                    const std::vector<PredictionResult>& predictions);
                            };

                            }  // namespace stock_predict
