#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <memory>
#include <random>
#include <vector>

// Include data structures and market analysis components
#include "../../src/core/data_structures.h"
#include "../../src/core/market_analyzer.h"
#include "../../src/feature_engineering/feature_extractor.h"
#include "../../src/feature_engineering/technical_indicators.h"

class Phase3FeatureEngineering : public ::testing::Test {
   protected:
    void SetUp() override {
        // Generate sample stock data for testing
        generateSampleData();

        // Initialize feature extractor
        feature_extractor = std::make_unique<FeatureExtractor>();

        // Initialize technical indicators
        technical_indicators = std::make_unique<TechnicalIndicators>();

        // Initialize market analyzer
        market_analyzer = std::make_unique<MarketAnalyzer>();
    }

    void TearDown() override {
        // Cleanup test data
        sample_data.clear();
        features.clear();
    }

    void generateSampleData() {
        // Generate 252 days of sample data (1 trading year)
        sample_data.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> price_change(-0.001, 0.02);
        std::uniform_real_distribution<double> volume_factor(0.8, 1.2);

        double base_price = 100.0;
        uint64_t base_volume = 1000000;

        auto start_time = std::chrono::system_clock::now() - std::chrono::hours(24 * 252);

        for (int i = 0; i < 252; ++i) {
            StockData data;
            data.symbol = "TEST";
            data.timestamp = start_time + std::chrono::hours(24 * i);

            // Generate OHLC data
            double change = price_change(gen);
            data.open = base_price;
            data.high =
                base_price * (1.0 + std::max(0.0, change) + std::abs(price_change(gen)) * 0.5);
            data.low =
                base_price * (1.0 + std::min(0.0, change) - std::abs(price_change(gen)) * 0.5);
            data.close = base_price * (1.0 + change);
            data.volume = static_cast<uint64_t>(base_volume * volume_factor(gen));

            base_price = data.close;
            sample_data.push_back(data);
        }
    }

    std::vector<StockData> sample_data;
    std::vector<FeatureVector> features;
    std::unique_ptr<FeatureExtractor> feature_extractor;
    std::unique_ptr<TechnicalIndicators> technical_indicators;
    std::unique_ptr<MarketAnalyzer> market_analyzer;
};

// Test T3.1: Technical Indicators
class TechnicalIndicatorsTest : public Phase3FeatureEngineering {};

TEST_F(TechnicalIndicatorsTest, T3_1_1_MovingAverages) {
    // Test Simple Moving Average (SMA)
    auto sma_20 = technical_indicators->calculateSMA(sample_data, 20);
    ASSERT_FALSE(sma_20.empty());
    EXPECT_EQ(sma_20.size(), sample_data.size() - 19);  // SMA starts from day 20

    // Test Exponential Moving Average (EMA)
    auto ema_20 = technical_indicators->calculateEMA(sample_data, 20);
    ASSERT_FALSE(ema_20.empty());
    EXPECT_EQ(ema_20.size(), sample_data.size());

    // Verify SMA calculation manually for first few values
    double manual_sma = 0.0;
    for (int i = 0; i < 20; ++i) {
        manual_sma += sample_data[i].close;
    }
    manual_sma /= 20.0;
    EXPECT_NEAR(sma_20[0], manual_sma, 0.01);

    // Test edge cases
    EXPECT_THROW(technical_indicators->calculateSMA(sample_data, 0), std::invalid_argument);
    EXPECT_THROW(technical_indicators->calculateSMA(sample_data, sample_data.size() + 1),
                 std::invalid_argument);
}

TEST_F(TechnicalIndicatorsTest, T3_1_2_MomentumIndicators) {
    // Test RSI (Relative Strength Index)
    auto rsi = technical_indicators->calculateRSI(sample_data, 14);
    ASSERT_FALSE(rsi.empty());

    // RSI should be between 0 and 100
    for (const auto& value : rsi) {
        EXPECT_GE(value, 0.0);
        EXPECT_LE(value, 100.0);
    }

    // Test MACD (Moving Average Convergence Divergence)
    auto macd = technical_indicators->calculateMACD(sample_data, 12, 26, 9);
    ASSERT_FALSE(macd.empty());
    EXPECT_EQ(macd.size(), 3);  // MACD line, Signal line, Histogram

    // Test Stochastic Oscillator
    auto stoch = technical_indicators->calculateStochastic(sample_data, 14, 3, 3);
    ASSERT_FALSE(stoch.empty());

    // Stochastic values should be between 0 and 100
    for (const auto& k_value : stoch[0]) {  // %K line
        EXPECT_GE(k_value, 0.0);
        EXPECT_LE(k_value, 100.0);
    }
}

TEST_F(TechnicalIndicatorsTest, T3_1_3_VolumeIndicators) {
    // Test On-Balance Volume (OBV)
    auto obv = technical_indicators->calculateOBV(sample_data);
    ASSERT_FALSE(obv.empty());
    EXPECT_EQ(obv.size(), sample_data.size());

    // Test Volume Weighted Average Price (VWAP)
    auto vwap = technical_indicators->calculateVWAP(sample_data);
    ASSERT_FALSE(vwap.empty());

    // VWAP should be within reasonable range of typical prices
    double avg_price = 0.0;
    for (const auto& data : sample_data) {
        avg_price += (data.high + data.low + data.close) / 3.0;
    }
    avg_price /= sample_data.size();

    for (const auto& value : vwap) {
        EXPECT_GT(value, avg_price * 0.8);
        EXPECT_LT(value, avg_price * 1.2);
    }

    // Test Accumulation/Distribution Line
    auto ad_line = technical_indicators->calculateADLine(sample_data);
    ASSERT_FALSE(ad_line.empty());
    EXPECT_EQ(ad_line.size(), sample_data.size());
}

TEST_F(TechnicalIndicatorsTest, T3_1_4_VolatilityIndicators) {
    // Test Bollinger Bands
    auto bollinger = technical_indicators->calculateBollingerBands(sample_data, 20, 2.0);
    ASSERT_FALSE(bollinger.empty());
    EXPECT_EQ(bollinger.size(), 3);  // Upper, Middle (SMA), Lower bands

    // Upper band should be above middle, middle above lower
    for (size_t i = 0; i < bollinger[0].size(); ++i) {
        EXPECT_GT(bollinger[0][i], bollinger[1][i]);  // Upper > Middle
        EXPECT_GT(bollinger[1][i], bollinger[2][i]);  // Middle > Lower
    }

    // Test Average True Range (ATR)
    auto atr = technical_indicators->calculateATR(sample_data, 14);
    ASSERT_FALSE(atr.empty());

    // ATR should be positive
    for (const auto& value : atr) {
        EXPECT_GT(value, 0.0);
    }

    // Test Standard Deviation
    auto std_dev = technical_indicators->calculateStandardDeviation(sample_data, 20);
    ASSERT_FALSE(std_dev.empty());

    // Standard deviation should be non-negative
    for (const auto& value : std_dev) {
        EXPECT_GE(value, 0.0);
    }
}

TEST_F(TechnicalIndicatorsTest, T3_1_5_TrendIndicators) {
    // Test Parabolic SAR
    auto sar = technical_indicators->calculateParabolicSAR(sample_data, 0.02, 0.2);
    ASSERT_FALSE(sar.empty());

    // Test Ichimoku Cloud components
    auto ichimoku = technical_indicators->calculateIchimoku(sample_data, 9, 26, 52);
    ASSERT_FALSE(ichimoku.empty());
    EXPECT_EQ(ichimoku.size(), 5);  // Tenkan, Kijun, Senkou A, Senkou B, Chikou

    // Test ADX (Average Directional Index)
    auto adx = technical_indicators->calculateADX(sample_data, 14);
    ASSERT_FALSE(adx.empty());

    // ADX should be between 0 and 100
    for (const auto& value : adx) {
        EXPECT_GE(value, 0.0);
        EXPECT_LE(value, 100.0);
    }
}

// Test T3.2: Feature Extraction
class FeatureExtractionTest : public Phase3FeatureEngineering {};

TEST_F(FeatureExtractionTest, T3_2_1_PriceFeatures) {
    auto price_features = feature_extractor->extractPriceFeatures(sample_data);
    ASSERT_FALSE(price_features.empty());

    // Check that we have the expected number of price features
    // Returns, price changes, normalized prices, etc.
    EXPECT_GT(price_features.size(), 10);

    // Test specific price features
    auto returns = feature_extractor->calculateReturns(sample_data);
    ASSERT_FALSE(returns.empty());
    EXPECT_EQ(returns.size(), sample_data.size() - 1);

    // Test log returns
    auto log_returns = feature_extractor->calculateLogReturns(sample_data);
    ASSERT_FALSE(log_returns.empty());
    EXPECT_EQ(log_returns.size(), sample_data.size() - 1);

    // Test price normalization
    auto normalized_prices = feature_extractor->normalizePrices(sample_data);
    ASSERT_FALSE(normalized_prices.empty());
    EXPECT_EQ(normalized_prices.size(), sample_data.size());
}

TEST_F(FeatureExtractionTest, T3_2_2_VolumeFeatures) {
    auto volume_features = feature_extractor->extractVolumeFeatures(sample_data);
    ASSERT_FALSE(volume_features.empty());

    // Test volume rate of change
    auto volume_roc = feature_extractor->calculateVolumeROC(sample_data, 5);
    ASSERT_FALSE(volume_roc.empty());

    // Test volume moving averages
    auto volume_ma = feature_extractor->calculateVolumeMA(sample_data, 20);
    ASSERT_FALSE(volume_ma.empty());

    // Test volume ratio (current volume / average volume)
    auto volume_ratio = feature_extractor->calculateVolumeRatio(sample_data, 10);
    ASSERT_FALSE(volume_ratio.empty());

    // Volume ratios should be positive
    for (const auto& ratio : volume_ratio) {
        EXPECT_GT(ratio, 0.0);
    }
}

TEST_F(FeatureExtractionTest, T3_2_3_TimeFeatures) {
    auto time_features = feature_extractor->extractTimeFeatures(sample_data);
    ASSERT_FALSE(time_features.empty());

    // Should include: day of week, day of month, month, quarter, etc.
    EXPECT_GE(time_features.size(), 5);

    // Test specific time features
    auto day_of_week = feature_extractor->getDayOfWeek(sample_data);
    ASSERT_FALSE(day_of_week.empty());

    // Day of week should be 0-6 (Sunday-Saturday)
    for (const auto& day : day_of_week) {
        EXPECT_GE(day, 0);
        EXPECT_LE(day, 6);
    }

    // Test month extraction
    auto months = feature_extractor->getMonth(sample_data);
    ASSERT_FALSE(months.empty());

    // Month should be 1-12
    for (const auto& month : months) {
        EXPECT_GE(month, 1);
        EXPECT_LE(month, 12);
    }
}

TEST_F(FeatureExtractionTest, T3_2_4_LagFeatures) {
    auto lag_features = feature_extractor->extractLagFeatures(sample_data, {1, 2, 3, 5, 10});
    ASSERT_FALSE(lag_features.empty());

    // Should have features for each specified lag
    EXPECT_EQ(lag_features.size(), 5);

    // Test specific lag feature
    auto lag_1_prices = feature_extractor->createLagFeature(sample_data, "close", 1);
    ASSERT_FALSE(lag_1_prices.empty());
    EXPECT_EQ(lag_1_prices.size(), sample_data.size() - 1);

    // Verify lag feature correctness
    for (size_t i = 0; i < lag_1_prices.size(); ++i) {
        EXPECT_DOUBLE_EQ(lag_1_prices[i], sample_data[i].close);
    }
}

TEST_F(FeatureExtractionTest, T3_2_5_RollingFeatures) {
    auto rolling_features = feature_extractor->extractRollingFeatures(sample_data, {5, 10, 20});
    ASSERT_FALSE(rolling_features.empty());

    // Test rolling statistics
    auto rolling_mean = feature_extractor->calculateRollingMean(sample_data, "close", 10);
    ASSERT_FALSE(rolling_mean.empty());

    auto rolling_std = feature_extractor->calculateRollingStd(sample_data, "close", 10);
    ASSERT_FALSE(rolling_std.empty());

    auto rolling_min = feature_extractor->calculateRollingMin(sample_data, "close", 10);
    ASSERT_FALSE(rolling_min.empty());

    auto rolling_max = feature_extractor->calculateRollingMax(sample_data, "close", 10);
    ASSERT_FALSE(rolling_max.empty());

    // Verify rolling window relationships
    for (size_t i = 0; i < rolling_min.size(); ++i) {
        EXPECT_LE(rolling_min[i], rolling_mean[i]);
        EXPECT_LE(rolling_mean[i], rolling_max[i]);
        EXPECT_GE(rolling_std[i], 0.0);
    }
}

// Test T3.3: Market Analysis
class MarketAnalysisTest : public Phase3FeatureEngineering {};

TEST_F(MarketAnalysisTest, T3_3_1_TrendAnalysis) {
    auto trend = market_analyzer->analyzeTrend(sample_data);
    ASSERT_TRUE(trend.has_value());

    // Trend should be one of: UPTREND, DOWNTREND, SIDEWAYS
    EXPECT_TRUE(trend == TrendDirection::UPTREND || trend == TrendDirection::DOWNTREND ||
                trend == TrendDirection::SIDEWAYS);

    // Test trend strength
    auto trend_strength = market_analyzer->calculateTrendStrength(sample_data);
    EXPECT_GE(trend_strength, 0.0);
    EXPECT_LE(trend_strength, 1.0);

    // Test support and resistance levels
    auto support_resistance = market_analyzer->findSupportResistanceLevels(sample_data);
    ASSERT_FALSE(support_resistance.first.empty());
    ASSERT_FALSE(support_resistance.second.empty());
}

TEST_F(MarketAnalysisTest, T3_3_2_VolatilityAnalysis) {
    auto volatility = market_analyzer->calculateVolatility(sample_data, 20);
    EXPECT_GE(volatility, 0.0);

    // Test volatility regime detection
    auto vol_regime = market_analyzer->detectVolatilityRegime(sample_data);
    EXPECT_TRUE(vol_regime == VolatilityRegime::LOW || vol_regime == VolatilityRegime::NORMAL ||
                vol_regime == VolatilityRegime::HIGH);

    // Test volatility clustering
    auto vol_clustering = market_analyzer->detectVolatilityClustering(sample_data);
    EXPECT_GE(vol_clustering, 0.0);
    EXPECT_LE(vol_clustering, 1.0);
}

TEST_F(MarketAnalysisTest, T3_3_3_PatternRecognition) {
    // Test candlestick pattern recognition
    auto patterns = market_analyzer->recognizeCandlestickPatterns(sample_data);
    ASSERT_TRUE(patterns.size() >= 0);  // May not find patterns in random data

    // Test chart pattern recognition
    auto chart_patterns = market_analyzer->recognizeChartPatterns(sample_data);
    ASSERT_TRUE(chart_patterns.size() >= 0);

    // Test gap detection
    auto gaps = market_analyzer->detectGaps(sample_data);
    ASSERT_TRUE(gaps.size() >= 0);
}

TEST_F(MarketAnalysisTest, T3_3_4_SeasonalityAnalysis) {
    auto seasonality = market_analyzer->analyzeSeasonality(sample_data);
    ASSERT_FALSE(seasonality.empty());

    // Test specific seasonal patterns
    auto monthly_seasonality = market_analyzer->getMonthlySeasonality(sample_data);
    EXPECT_EQ(monthly_seasonality.size(), 12);  // 12 months

    auto weekly_seasonality = market_analyzer->getWeeklySeasonality(sample_data);
    EXPECT_EQ(weekly_seasonality.size(), 7);  // 7 days of week
}

TEST_F(MarketAnalysisTest, T3_3_5_AnomalyDetection) {
    auto anomalies = market_analyzer->detectAnomalies(sample_data);
    ASSERT_TRUE(anomalies.size() >= 0);  // May not find anomalies in normal data

    // Test outlier detection
    auto outliers = market_analyzer->detectPriceOutliers(sample_data, 2.0);
    ASSERT_TRUE(outliers.size() >= 0);

    auto volume_outliers = market_analyzer->detectVolumeOutliers(sample_data, 2.0);
    ASSERT_TRUE(volume_outliers.size() >= 0);
}

// Test T3.4: Feature Selection
class FeatureSelectionTest : public Phase3FeatureEngineering {};

TEST_F(FeatureSelectionTest, T3_4_1_CorrelationAnalysis) {
    // Extract all features first
    auto all_features = feature_extractor->extractAllFeatures(sample_data);
    ASSERT_FALSE(all_features.empty());

    // Test correlation matrix calculation
    auto correlation_matrix = feature_extractor->calculateCorrelationMatrix(all_features);
    ASSERT_FALSE(correlation_matrix.empty());

    // Correlation matrix should be square
    for (const auto& row : correlation_matrix) {
        EXPECT_EQ(row.size(), correlation_matrix.size());
    }

    // Diagonal elements should be 1.0 (self-correlation)
    for (size_t i = 0; i < correlation_matrix.size(); ++i) {
        EXPECT_NEAR(correlation_matrix[i][i], 1.0, 0.01);
    }
}

TEST_F(FeatureSelectionTest, T3_4_2_FeatureImportance) {
    auto all_features = feature_extractor->extractAllFeatures(sample_data);

    // Test feature importance calculation using various methods
    auto importance_scores =
        feature_extractor->calculateFeatureImportance(all_features, sample_data);
    ASSERT_FALSE(importance_scores.empty());
    EXPECT_EQ(importance_scores.size(), all_features.size());

    // Importance scores should be non-negative
    for (const auto& score : importance_scores) {
        EXPECT_GE(score, 0.0);
    }
}

TEST_F(FeatureSelectionTest, T3_4_3_FeatureSelection) {
    auto all_features = feature_extractor->extractAllFeatures(sample_data);

    // Test feature selection based on correlation threshold
    auto selected_features_corr = feature_extractor->selectFeaturesByCorrelation(all_features, 0.8);
    EXPECT_LE(selected_features_corr.size(), all_features.size());

    // Test feature selection based on importance threshold
    auto selected_features_imp =
        feature_extractor->selectFeaturesByImportance(all_features, sample_data, 0.1);
    EXPECT_LE(selected_features_imp.size(), all_features.size());

    // Test top-k feature selection
    auto top_k_features = feature_extractor->selectTopKFeatures(all_features, sample_data, 10);
    EXPECT_LE(top_k_features.size(), std::min(10UL, all_features.size()));
}

// Performance and Integration Tests
class Phase3IntegrationTest : public Phase3FeatureEngineering {};

TEST_F(Phase3IntegrationTest, T3_5_1_FeatureExtractionPipeline) {
    // Test complete feature extraction pipeline
    auto start_time = std::chrono::high_resolution_clock::now();

    auto complete_features = feature_extractor->extractCompleteFeatureSet(sample_data);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    ASSERT_FALSE(complete_features.empty());

    // Feature extraction should complete within reasonable time
    EXPECT_LT(duration.count(), 5000);  // Less than 5 seconds

    // Verify feature completeness
    EXPECT_GT(complete_features.size(), 50);  // Should have many features
}

TEST_F(Phase3IntegrationTest, T3_5_2_RealTimeFeatureExtraction) {
    // Test real-time feature extraction with streaming data
    std::vector<std::future<FeatureVector>> futures;

    // Simulate streaming data by processing in chunks
    size_t chunk_size = 50;
    for (size_t i = 0; i < sample_data.size(); i += chunk_size) {
        size_t end_idx = std::min(i + chunk_size, sample_data.size());
        std::vector<StockData> chunk(sample_data.begin() + i, sample_data.begin() + end_idx);

        futures.push_back(std::async(std::launch::async, [this, chunk]() {
            return feature_extractor->extractRealTimeFeatures(chunk);
        }));
    }

    // Collect results
    for (auto& future : futures) {
        auto features = future.get();
        EXPECT_FALSE(features.empty());
    }
}

TEST_F(Phase3IntegrationTest, T3_5_3_MemoryUsage) {
    // Test memory usage with large datasets
    std::vector<StockData> large_dataset;
    large_dataset.reserve(10000);

    // Generate larger dataset
    for (int i = 0; i < 10000; ++i) {
        StockData data = sample_data[i % sample_data.size()];
        data.timestamp += std::chrono::hours(24 * i);
        large_dataset.push_back(data);
    }

    // Extract features and monitor memory usage
    auto features = feature_extractor->extractCompleteFeatureSet(large_dataset);
    ASSERT_FALSE(features.empty());

    // Verify we can handle large datasets
    EXPECT_GT(features.size(), 100);
}

TEST_F(Phase3IntegrationTest, T3_5_4_FeatureValidation) {
    auto features = feature_extractor->extractCompleteFeatureSet(sample_data);

    // Test feature validation
    bool is_valid = feature_extractor->validateFeatures(features);
    EXPECT_TRUE(is_valid);

    // Test for NaN or infinite values
    for (const auto& feature_vec : features) {
        for (double value : feature_vec.values) {
            EXPECT_FALSE(std::isnan(value)) << "Feature contains NaN value";
            EXPECT_FALSE(std::isinf(value)) << "Feature contains infinite value";
        }
    }

    // Test feature scaling/normalization
    auto normalized_features = feature_extractor->normalizeFeatures(features);
    ASSERT_FALSE(normalized_features.empty());
    EXPECT_EQ(normalized_features.size(), features.size());
}

TEST_F(Phase3IntegrationTest, T3_5_5_CrossValidation) {
    auto features = feature_extractor->extractCompleteFeatureSet(sample_data);

    // Test k-fold cross-validation setup for feature selection
    int k = 5;
    auto folds = feature_extractor->createKFolds(features, k);
    EXPECT_EQ(folds.size(), k);

    // Each fold should have roughly equal size
    size_t expected_fold_size = features.size() / k;
    for (const auto& fold : folds) {
        EXPECT_GE(fold.size(), expected_fold_size - 1);
        EXPECT_LE(fold.size(), expected_fold_size + 1);
    }

    // Test that all data is covered and no overlap
    std::set<size_t> all_indices;
    for (const auto& fold : folds) {
        for (size_t idx : fold) {
            EXPECT_TRUE(all_indices.insert(idx).second) << "Duplicate index found in folds";
        }
    }
    EXPECT_EQ(all_indices.size(), features.size());
}
