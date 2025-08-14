#include <gtest/gtest.h>
#include <memory>
#include "stock_predict/data.hpp"
#include "stock_predict/models.hpp"
#include "stock_predict/stock_predictor.hpp"

// Phase 2: Core Prediction Engine Tests
// Test T2.1.1-T2.4.5: Core Prediction Engine Tests

namespace phase2 {
namespace core_engine {

class CorePredictionEngineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Set up test configuration
        config_.model_type = "ensemble";
        config_.sequence_length = 60;
        config_.prediction_horizon = 1;
        config_.batch_size = 32;
        config_.learning_rate = 0.001;
        config_.use_gpu = false;  // Use CPU for testing

        // Create sample market data
        setupSampleData();
    }

    void setupSampleData() {
        auto now = std::chrono::system_clock::now();
        sample_data_.clear();

        // Generate 100 days of sample data
        for (int i = 0; i < 100; ++i) {
            stock_predict::MarketData data;
            data.symbol = "TEST";
            data.timestamp = now - std::chrono::hours(24 * (100 - i));
            data.open = 100.0 + i * 0.1;
            data.high = data.open + 2.0;
            data.low = data.open - 1.5;
            data.close = data.open + (i % 2 == 0 ? 0.5 : -0.3);
            data.volume = 1000000 + i * 1000;
            data.adjusted_close = data.close;

            sample_data_.push_back(data);
        }
    }

    stock_predict::PredictorConfig config_;
    std::vector<stock_predict::MarketData> sample_data_;
};

// Test T2.1.1: Interface contract validation
TEST_F(CorePredictionEngineTest, InterfaceContractValidation) {
    auto predictor = stock_predict::create_predictor("TEST", config_);
    ASSERT_NE(predictor, nullptr) << "Predictor should be created successfully";

    // Test interface methods exist
    EXPECT_NO_THROW(predictor->train(sample_data_, 1)) << "Train method should be callable";

    EXPECT_NO_THROW(predictor->predict_next_day(sample_data_))
        << "Predict next day method should be callable";

    EXPECT_NO_THROW(predictor->predict_multi_day(sample_data_, 5))
        << "Predict multi day method should be callable";

    EXPECT_NO_THROW(predictor->get_performance_metrics())
        << "Get performance metrics method should be callable";
}

// Test T2.1.2: Factory pattern functionality
TEST_F(CorePredictionEngineTest, FactoryPatternFunctionality) {
    // Test LSTM creation
    config_.model_type = "lstm";
    auto lstm_predictor = stock_predict::create_predictor("TEST", config_);
    EXPECT_NE(lstm_predictor, nullptr) << "LSTM predictor should be created";

    // Test Transformer creation
    config_.model_type = "transformer";
    auto transformer_predictor = stock_predict::create_predictor("TEST", config_);
    EXPECT_NE(transformer_predictor, nullptr) << "Transformer predictor should be created";

    // Test Ensemble creation
    config_.model_type = "ensemble";
    auto ensemble_predictor = stock_predict::create_predictor("TEST", config_);
    EXPECT_NE(ensemble_predictor, nullptr) << "Ensemble predictor should be created";

    // Test invalid model type
    config_.model_type = "invalid_model";
    EXPECT_THROW(stock_predict::create_predictor("TEST", config_), std::invalid_argument)
        << "Invalid model type should throw exception";
}

// Test T2.1.3: Market data structure integrity
TEST_F(CorePredictionEngineTest, MarketDataStructureIntegrity) {
    EXPECT_FALSE(sample_data_.empty()) << "Sample data should not be empty";
    EXPECT_EQ(sample_data_.size(), 100) << "Should have 100 data points";

    for (const auto& data : sample_data_) {
        EXPECT_FALSE(data.symbol.empty()) << "Symbol should not be empty";
        EXPECT_GT(data.open, 0) << "Open price should be positive";
        EXPECT_GT(data.high, 0) << "High price should be positive";
        EXPECT_GT(data.low, 0) << "Low price should be positive";
        EXPECT_GT(data.close, 0) << "Close price should be positive";
        EXPECT_GT(data.volume, 0) << "Volume should be positive";
        EXPECT_GE(data.high, data.open) << "High should be >= open";
        EXPECT_GE(data.high, data.close) << "High should be >= close";
        EXPECT_LE(data.low, data.open) << "Low should be <= open";
        EXPECT_LE(data.low, data.close) << "Low should be <= close";
    }
}

// Test T2.1.4: Prediction result validation
TEST_F(CorePredictionEngineTest, PredictionResultValidation) {
    auto predictor = stock_predict::create_predictor("TEST", config_);

    // Train with minimal epochs for testing
    predictor->train(sample_data_, 1);

    // Test single day prediction
    auto single_result = predictor->predict_next_day(sample_data_);
    EXPECT_GT(single_result.price, 0) << "Predicted price should be positive";
    EXPECT_GE(single_result.confidence, 0) << "Confidence should be non-negative";
    EXPECT_LE(single_result.confidence, 1) << "Confidence should be <= 1";

    // Test multi-day prediction
    auto multi_results = predictor->predict_multi_day(sample_data_, 3);
    EXPECT_EQ(multi_results.size(), 3) << "Should return 3 predictions";

    for (const auto& result : multi_results) {
        EXPECT_GT(result.price, 0) << "Each predicted price should be positive";
        EXPECT_GE(result.confidence, 0) << "Each confidence should be non-negative";
        EXPECT_LE(result.confidence, 1) << "Each confidence should be <= 1";
    }
}

// Test T2.1.5: Configuration system testing
TEST_F(CorePredictionEngineTest, ConfigurationSystemTesting) {
    // Test configuration validation
    config_.sequence_length = 0;
    EXPECT_THROW(stock_predict::create_predictor("TEST", config_), std::invalid_argument)
        << "Zero sequence length should throw exception";

    config_.sequence_length = 60;
    config_.batch_size = 0;
    EXPECT_THROW(stock_predict::create_predictor("TEST", config_), std::invalid_argument)
        << "Zero batch size should throw exception";

    config_.batch_size = 32;
    config_.learning_rate = -0.1;
    EXPECT_THROW(stock_predict::create_predictor("TEST", config_), std::invalid_argument)
        << "Negative learning rate should throw exception";

    // Test valid configuration
    config_.learning_rate = 0.001;
    EXPECT_NO_THROW(stock_predict::create_predictor("TEST", config_))
        << "Valid configuration should not throw exception";
}

// Test T2.2.1: PyTorch integration validation
TEST_F(CorePredictionEngineTest, PyTorchIntegrationValidation) {
    config_.model_type = "lstm";
    auto predictor = stock_predict::create_predictor("TEST", config_);

    // Test that PyTorch tensors can be created and used
    EXPECT_NO_THROW(predictor->train(sample_data_, 1))
        << "LSTM training should work with PyTorch backend";

    // Test prediction works
    EXPECT_NO_THROW(predictor->predict_next_day(sample_data_))
        << "LSTM prediction should work with PyTorch backend";
}

// Test T2.3.1: Attention mechanism validation
TEST_F(CorePredictionEngineTest, AttentionMechanismValidation) {
    config_.model_type = "transformer";
    auto predictor = stock_predict::create_predictor("TEST", config_);

    // Test transformer-specific functionality
    EXPECT_NO_THROW(predictor->train(sample_data_, 1)) << "Transformer training should work";

    auto result = predictor->predict_next_day(sample_data_);
    EXPECT_GT(result.price, 0) << "Transformer should produce valid predictions";
}

// Test T2.4.1: Ensemble architecture validation
TEST_F(CorePredictionEngineTest, EnsembleArchitectureValidation) {
    config_.model_type = "ensemble";
    auto predictor = stock_predict::create_predictor("TEST", config_);

    // Test ensemble functionality
    EXPECT_NO_THROW(predictor->train(sample_data_, 1)) << "Ensemble training should work";

    auto result = predictor->predict_next_day(sample_data_);
    EXPECT_GT(result.price, 0) << "Ensemble should produce valid predictions";

    // Ensemble should generally have higher confidence than individual models
    // (This is a heuristic test)
    EXPECT_GE(result.confidence, 0.1) << "Ensemble should have reasonable confidence";
}

}  // namespace core_engine
}  // namespace phase2
