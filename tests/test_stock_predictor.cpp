#include <gtest/gtest.h>
#include "stock_predict/stock_predictor.hpp"
#include "stock_predict/models.hpp"
#include <vector>
#include <chrono>

class StockPredictorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create sample market data
        auto now = std::chrono::system_clock::now();
        for (int i = 0; i < 100; ++i) {
            stock_predict::MarketData data;
            data.symbol = "TEST";
            data.timestamp = now - std::chrono::hours(24 * (100 - i));
            data.open = 100.0 + i * 0.1;
            data.high = data.open + 2.0;
            data.low = data.open - 1.5;
            data.close = data.open + 0.5;
            data.volume = 1000000;
            data.adjusted_close = data.close;
            sample_data_.push_back(data);
        }
    }

    std::vector<stock_predict::MarketData> sample_data_;
};

TEST_F(StockPredictorTest, CreateLSTMPredictor) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_NE(predictor, nullptr);
    
    // Test that we can make a prediction structure (even if not trained)
    EXPECT_NO_THROW({
        // This would require a trained model, so we'll just test creation
        auto metrics = predictor->get_performance_metrics();
    });
}

TEST_F(StockPredictorTest, CreateTransformerPredictor) {
    stock_predict::PredictorConfig config;
    config.model_type = "transformer";
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_NE(predictor, nullptr);
}

TEST_F(StockPredictorTest, CreateEnsemblePredictor) {
    stock_predict::PredictorConfig config;
    config.model_type = "ensemble";
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_NE(predictor, nullptr);
}

TEST_F(StockPredictorTest, InvalidModelType) {
    stock_predict::PredictorConfig config;
    config.model_type = "invalid_model";
    
    EXPECT_THROW(
        stock_predict::create_predictor("TEST", config),
        std::invalid_argument
    );
}

TEST_F(StockPredictorTest, TrainingDataValidation) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 50;
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    
    // Test with insufficient data
    std::vector<stock_predict::MarketData> small_data(sample_data_.begin(), sample_data_.begin() + 10);
    EXPECT_FALSE(predictor->train(small_data, 1));
    
    // Test with sufficient data
    EXPECT_TRUE(predictor->train(sample_data_, 5)); // Just 5 epochs for speed
}

TEST_F(StockPredictorTest, PredictionResultStructure) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 50;
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    
    // Train with minimal epochs
    ASSERT_TRUE(predictor->train(sample_data_, 2));
    
    // Make prediction
    std::vector<stock_predict::MarketData> recent_data(
        sample_data_.end() - 60, sample_data_.end());
    
    auto result = predictor->predict_next_day(recent_data);
    
    // Validate prediction result structure
    EXPECT_GT(result.price, 0.0);
    EXPECT_GE(result.confidence, 0.0);
    EXPECT_LE(result.confidence, 1.0);
    EXPECT_GE(result.volatility, 0.0);
    EXPECT_NE(result.timestamp.time_since_epoch().count(), 0);
}

TEST_F(StockPredictorTest, MultiDayPrediction) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 50;
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_TRUE(predictor->train(sample_data_, 2));
    
    std::vector<stock_predict::MarketData> recent_data(
        sample_data_.end() - 60, sample_data_.end());
    
    auto results = predictor->predict_multi_day(recent_data, 5);
    
    EXPECT_EQ(results.size(), 5);
    
    // Check that predictions are reasonable
    for (const auto& result : results) {
        EXPECT_GT(result.price, 0.0);
        EXPECT_GE(result.confidence, 0.0);
        EXPECT_LE(result.confidence, 1.0);
    }
}

TEST_F(StockPredictorTest, ModelSaveLoad) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 50;
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_TRUE(predictor->train(sample_data_, 2));
    
    // Save model
    std::string model_path = "/tmp/test_model.pt";
    EXPECT_TRUE(predictor->save_model(model_path));
    
    // Create new predictor and load model
    auto new_predictor = stock_predict::create_predictor("TEST", config);
    EXPECT_TRUE(new_predictor->load_model(model_path));
    
    // Test that loaded model can make predictions
    std::vector<stock_predict::MarketData> recent_data(
        sample_data_.end() - 60, sample_data_.end());
    
    EXPECT_NO_THROW({
        auto result = new_predictor->predict_next_day(recent_data);
        EXPECT_GT(result.price, 0.0);
    });
}

// Performance test
TEST_F(StockPredictorTest, PredictionPerformance) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 50;
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_TRUE(predictor->train(sample_data_, 2));
    
    std::vector<stock_predict::MarketData> recent_data(
        sample_data_.end() - 60, sample_data_.end());
    
    // Time multiple predictions
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        auto result = predictor->predict_next_day(recent_data);
        EXPECT_GT(result.price, 0.0);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Average should be less than 10ms per prediction (very generous for testing)
    double avg_microseconds = duration.count() / 100.0;
    EXPECT_LT(avg_microseconds, 10000.0) << "Prediction too slow: " << avg_microseconds << " μs";
    
    std::cout << "Average prediction time: " << avg_microseconds << " μs" << std::endl;
}

// Edge cases
TEST_F(StockPredictorTest, EmptyDataHandling) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    
    std::vector<stock_predict::MarketData> empty_data;
    EXPECT_FALSE(predictor->train(empty_data, 1));
}

TEST_F(StockPredictorTest, InsufficientRecentData) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 50;
    
    auto predictor = stock_predict::create_predictor("TEST", config);
    ASSERT_TRUE(predictor->train(sample_data_, 2));
    
    // Try to predict with insufficient recent data
    std::vector<stock_predict::MarketData> insufficient_data(sample_data_.begin(), sample_data_.begin() + 10);
    
    EXPECT_THROW(
        predictor->predict_next_day(insufficient_data),
        std::runtime_error
    );
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
