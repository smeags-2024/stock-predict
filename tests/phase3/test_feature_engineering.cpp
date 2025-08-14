#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <future>
#include <memory>
#include <random>
#include <vector>
#include "stock_predict/features.hpp"
#include "stock_predict/models.hpp"
#include "stock_predict/stock_predictor.hpp"

using namespace stock_predict;

// Simple test fixture for feature engineering
class FeatureEngineeringTest : public testing::Test {
protected:
    void SetUp() override {
        // Create sample data for testing (30 days)
        sample_data.reserve(30);
        auto start_time = std::chrono::system_clock::now() - std::chrono::hours(24 * 30);
        
        double base_price = 100.0;
        for (int i = 0; i < 30; ++i) {
            MarketData data;
            data.symbol = "TEST";
            data.timestamp = start_time + std::chrono::hours(24 * i);
            data.open = base_price;
            data.high = base_price * 1.02;
            data.low = base_price * 0.98;
            data.close = base_price * (1.0 + (i % 3 - 1) * 0.01);  // Simple pattern
            data.volume = 1000000.0;
            data.adjusted_close = data.close;
            
            base_price = data.close;
            sample_data.push_back(data);
        }
    }

    std::vector<MarketData> sample_data;
};

// Test basic technical indicators
TEST_F(FeatureEngineeringTest, BasicTechnicalIndicators) {
    // Extract close prices
    std::vector<double> close_prices;
    for (const auto& data : sample_data) {
        close_prices.push_back(data.close);
    }
    
    // Test SMA
    auto sma_5 = TechnicalIndicators::sma(close_prices, 5);
    EXPECT_FALSE(sma_5.empty());
    EXPECT_EQ(sma_5.size(), close_prices.size() - 4);
    
    // Test EMA  
    auto ema_5 = TechnicalIndicators::ema(close_prices, 5);
    EXPECT_FALSE(ema_5.empty());
    EXPECT_EQ(ema_5.size(), close_prices.size());
    
    // Test RSI
    auto rsi_14 = TechnicalIndicators::rsi(close_prices, 14);
    EXPECT_FALSE(rsi_14.empty());
    
    // Test Bollinger Bands
    auto [upper, middle, lower] = TechnicalIndicators::bollinger_bands(close_prices, 10, 2.0);
    EXPECT_FALSE(upper.empty());
    EXPECT_FALSE(middle.empty());
    EXPECT_FALSE(lower.empty());
    EXPECT_EQ(upper.size(), middle.size());
    EXPECT_EQ(middle.size(), lower.size());
}

// Test MACD indicator
TEST_F(FeatureEngineeringTest, MACDIndicator) {
    std::vector<double> close_prices;
    for (const auto& data : sample_data) {
        close_prices.push_back(data.close);
    }
    
    auto [macd_line, signal_line, histogram] = TechnicalIndicators::macd(close_prices, 12, 26, 9);
    EXPECT_FALSE(macd_line.empty());
    EXPECT_FALSE(signal_line.empty());
    EXPECT_FALSE(histogram.empty());
    EXPECT_EQ(macd_line.size(), signal_line.size());
    EXPECT_EQ(signal_line.size(), histogram.size());
}

// Test stochastic oscillator
TEST_F(FeatureEngineeringTest, StochasticOscillator) {
    std::vector<double> high_prices, low_prices, close_prices;
    for (const auto& data : sample_data) {
        high_prices.push_back(data.high);
        low_prices.push_back(data.low);
        close_prices.push_back(data.close);
    }
    
    auto [k_values, d_values] = TechnicalIndicators::stochastic(high_prices, low_prices, close_prices, 14, 3);
    EXPECT_FALSE(k_values.empty());
    EXPECT_FALSE(d_values.empty());
    
    // Check that stochastic values are within expected range [0, 100]
    for (double k : k_values) {
        EXPECT_GE(k, 0.0);
        EXPECT_LE(k, 100.0);
    }
    for (double d : d_values) {
        EXPECT_GE(d, 0.0);
        EXPECT_LE(d, 100.0);
    }
}

// Test ATR (Average True Range)
TEST_F(FeatureEngineeringTest, AveragetrueRange) {
    std::vector<double> high_prices, low_prices, close_prices;
    for (const auto& data : sample_data) {
        high_prices.push_back(data.high);
        low_prices.push_back(data.low);
        close_prices.push_back(data.close);
    }
    
    auto atr_values = TechnicalIndicators::atr(high_prices, low_prices, close_prices, 14);
    EXPECT_FALSE(atr_values.empty());
    
    // ATR should be positive
    for (double atr : atr_values) {
        EXPECT_GE(atr, 0.0);
    }
}

// Test error conditions
TEST_F(FeatureEngineeringTest, ErrorConditions) {
    std::vector<double> close_prices;
    for (const auto& data : sample_data) {
        close_prices.push_back(data.close);
    }
    
    // Test invalid periods
    EXPECT_THROW(TechnicalIndicators::sma(close_prices, 0), std::invalid_argument);
    EXPECT_THROW(TechnicalIndicators::sma(close_prices, close_prices.size() + 1), std::invalid_argument);
    EXPECT_THROW(TechnicalIndicators::ema(close_prices, 0), std::invalid_argument);
    EXPECT_THROW(TechnicalIndicators::rsi(close_prices, 0), std::invalid_argument);
    
    // Test empty data
    std::vector<double> empty_data;
    EXPECT_THROW(TechnicalIndicators::sma(empty_data, 5), std::invalid_argument);
    EXPECT_THROW(TechnicalIndicators::ema(empty_data, 5), std::invalid_argument);
    EXPECT_THROW(TechnicalIndicators::rsi(empty_data, 14), std::invalid_argument);
}
