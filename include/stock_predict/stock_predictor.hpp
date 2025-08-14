#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace stock_predict {

/**
 * @brief Structure representing a price prediction with confidence metrics
 */
struct PredictionResult {
    double price;                                     ///< Predicted price
    double confidence;                                ///< Confidence interval (0-1)
    double volatility;                                ///< Predicted volatility
    std::chrono::system_clock::time_point timestamp;  ///< Prediction timestamp

    // Risk metrics
    double value_at_risk_95;  ///< 95% Value at Risk
    double expected_return;   ///< Expected return
    double sharpe_ratio;      ///< Sharpe ratio
};

/**
 * @brief Market data point structure
 */
struct MarketData {
    std::string symbol;
    std::chrono::system_clock::time_point timestamp;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double adjusted_close;
};

/**
 * @brief Configuration for the stock predictor
 */
struct PredictorConfig {
    // Model parameters
    std::string model_path;
    std::string model_type = "lstm";  ///< Model type: lstm, gru, transformer
    int sequence_length = 60;         ///< Input sequence length
    int prediction_horizon = 1;       ///< Number of days to predict
    int batch_size = 32;              ///< Training batch size
    double learning_rate = 0.001;     ///< Learning rate for optimization

    // Data parameters
    std::vector<std::string> features = {"close", "volume", "rsi", "macd",
                                         "bb"};  ///< Features to use
    bool normalize_data = true;                  ///< Whether to normalize input data
    double train_test_split = 0.8;               ///< Train/test split ratio

    // Risk parameters
    double confidence_level = 0.95;  ///< Confidence level for VaR
    int var_window = 252;            ///< Window for VaR calculation (trading days)

    // Performance parameters
    bool use_gpu = false;  ///< Whether to use GPU acceleration
    int num_threads = 0;   ///< Number of threads (0 = auto)
};

/**
 * @brief Abstract base class for stock price predictors
 */
class IStockPredictor {
   public:
    virtual ~IStockPredictor() = default;

    /**
     * @brief Load a pre-trained model
     * @param model_path Path to the model file
     * @return true if successful, false otherwise
     */
    virtual bool load_model(const std::string& model_path) = 0;

    /**
     * @brief Train the model on historical data
     * @param data Historical market data
     * @param epochs Number of training epochs
     * @return true if successful, false otherwise
     */
    virtual bool train(const std::vector<MarketData>& data, int epochs = 100) = 0;

    /**
     * @brief Predict next day's price
     * @param recent_data Recent market data for prediction
     * @return Prediction result with confidence metrics
     */
    virtual PredictionResult predict_next_day(const std::vector<MarketData>& recent_data) = 0;

    /**
     * @brief Predict multiple days ahead
     * @param recent_data Recent market data for prediction
     * @param days Number of days to predict
     * @return Vector of prediction results
     */
    virtual std::vector<PredictionResult> predict_multi_day(
        const std::vector<MarketData>& recent_data, int days) = 0;

    /**
     * @brief Get model performance metrics
     * @return Performance metrics as key-value pairs
     */
    virtual std::vector<std::pair<std::string, double>> get_performance_metrics() const = 0;

    /**
     * @brief Save the trained model
     * @param model_path Path where to save the model
     * @return true if successful, false otherwise
     */
    virtual bool save_model(const std::string& model_path) const = 0;
};

/**
 * @brief Factory function to create stock predictors
 * @param symbol Stock symbol (e.g., "AAPL")
 * @param config Predictor configuration
 * @return Unique pointer to the predictor instance
 */
std::unique_ptr<IStockPredictor> create_predictor(const std::string& symbol,
                                                  const PredictorConfig& config = {});

}  // namespace stock_predict
