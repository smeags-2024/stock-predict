#include <benchmark/benchmark.h>
#include "stock_predict/stock_predictor.hpp"
#include "stock_predict/features.hpp"
#include "stock_predict/risk.hpp"
#include <vector>
#include <random>
#include <chrono>

// Helper function to generate sample data
std::vector<stock_predict::MarketData> generate_benchmark_data(int size) {
    std::vector<stock_predict::MarketData> data;
    auto now = std::chrono::system_clock::now();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> price_dist(100.0, 10.0);
    std::normal_distribution<> volume_dist(1000000.0, 100000.0);
    
    for (int i = 0; i < size; ++i) {
        stock_predict::MarketData point;
        point.symbol = "BENCH";
        point.timestamp = now - std::chrono::hours(24 * (size - i));
        point.close = std::max(1.0, price_dist(gen));
        point.open = point.close * (1.0 + std::normal_distribution<>(0.0, 0.01)(gen));
        point.high = std::max(point.open, point.close) * 1.02;
        point.low = std::min(point.open, point.close) * 0.98;
        point.volume = std::max(1000.0, volume_dist(gen));
        point.adjusted_close = point.close;
        data.push_back(point);
    }
    
    return data;
}

// Benchmark prediction latency
static void BM_PredictionLatency(benchmark::State& state) {
    // Setup
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 60;
    
    auto predictor = stock_predict::create_predictor("BENCH", config);
    auto data = generate_benchmark_data(200);
    
    // Quick training (minimal epochs for benchmarking)
    predictor->train(data, 2);
    
    std::vector<stock_predict::MarketData> recent_data(data.end() - 70, data.end());
    
    // Benchmark prediction
    for (auto _ : state) {
        auto result = predictor->predict_next_day(recent_data);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_PredictionLatency)->Unit(benchmark::kMicrosecond);

// Benchmark feature extraction
static void BM_FeatureExtraction(benchmark::State& state) {
    auto data = generate_benchmark_data(state.range(0));
    
    for (auto _ : state) {
        auto features = stock_predict::FeatureEngineer::extract_features(data);
        benchmark::DoNotOptimize(features);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_FeatureExtraction)->Range(100, 10000)->Complexity();

// Benchmark technical indicators
static void BM_TechnicalIndicators(benchmark::State& state) {
    std::vector<double> prices;
    for (int i = 0; i < state.range(0); ++i) {
        prices.push_back(100.0 + std::sin(i * 0.1) * 10.0);
    }
    
    for (auto _ : state) {
        auto rsi = stock_predict::TechnicalIndicators::rsi(prices);
        auto sma = stock_predict::TechnicalIndicators::sma(prices, 20);
        auto ema = stock_predict::TechnicalIndicators::ema(prices, 20);
        benchmark::DoNotOptimize(rsi);
        benchmark::DoNotOptimize(sma);
        benchmark::DoNotOptimize(ema);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_TechnicalIndicators)->Range(100, 5000)->Complexity();

// Benchmark VaR calculation
static void BM_VaRCalculation(benchmark::State& state) {
    std::vector<double> returns;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.001, 0.02);
    
    for (int i = 0; i < state.range(0); ++i) {
        returns.push_back(dist(gen));
    }
    
    for (auto _ : state) {
        auto var_95 = stock_predict::RiskManager::calculate_var(returns, 0.95);
        auto var_99 = stock_predict::RiskManager::calculate_var(returns, 0.99);
        benchmark::DoNotOptimize(var_95);
        benchmark::DoNotOptimize(var_99);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_VaRCalculation)->Range(100, 5000)->Complexity();

// Benchmark multi-day predictions
static void BM_MultiDayPrediction(benchmark::State& state) {
    stock_predict::PredictorConfig config;
    config.model_type = "lstm";
    config.sequence_length = 60;
    
    auto predictor = stock_predict::create_predictor("BENCH", config);
    auto data = generate_benchmark_data(200);
    
    // Quick training
    predictor->train(data, 2);
    
    std::vector<stock_predict::MarketData> recent_data(data.end() - 70, data.end());
    int prediction_days = state.range(0);
    
    for (auto _ : state) {
        auto results = predictor->predict_multi_day(recent_data, prediction_days);
        benchmark::DoNotOptimize(results);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MultiDayPrediction)->Range(1, 30)->Complexity();

// Benchmark data processing throughput
static void BM_DataProcessingThroughput(benchmark::State& state) {
    auto data = generate_benchmark_data(state.range(0));
    
    for (auto _ : state) {
        auto cleaned = stock_predict::DataProcessor::clean_data(data);
        benchmark::DoNotOptimize(cleaned);
    }
    
    state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(stock_predict::MarketData));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_DataProcessingThroughput)->Range(1000, 100000);

// Benchmark memory allocation patterns
static void BM_MemoryAllocation(benchmark::State& state) {
    int num_predictions = state.range(0);
    
    for (auto _ : state) {
        std::vector<stock_predict::PredictionResult> results;
        results.reserve(num_predictions);
        
        for (int i = 0; i < num_predictions; ++i) {
            stock_predict::PredictionResult result;
            result.price = 100.0 + i;
            result.confidence = 0.8;
            result.volatility = 0.02;
            result.timestamp = std::chrono::system_clock::now();
            results.push_back(result);
        }
        
        benchmark::DoNotOptimize(results);
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MemoryAllocation)->Range(100, 10000);

// Benchmark ensemble prediction
static void BM_EnsemblePrediction(benchmark::State& state) {
    stock_predict::PredictorConfig config;
    config.model_type = "ensemble";
    config.sequence_length = 60;
    
    auto predictor = stock_predict::create_predictor("BENCH", config);
    auto data = generate_benchmark_data(200);
    
    // Quick training
    predictor->train(data, 2);
    
    std::vector<stock_predict::MarketData> recent_data(data.end() - 70, data.end());
    
    for (auto _ : state) {
        auto result = predictor->predict_next_day(recent_data);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EnsemblePrediction)->Unit(benchmark::kMicrosecond);

// Benchmark portfolio optimization
static void BM_PortfolioOptimization(benchmark::State& state) {
    int num_assets = state.range(0);
    int num_periods = 252; // 1 year of daily data
    
    // Generate random returns
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.001, 0.02);
    
    std::vector<std::vector<double>> returns(num_assets);
    std::vector<std::string> asset_names;
    
    for (int i = 0; i < num_assets; ++i) {
        asset_names.push_back("ASSET" + std::to_string(i));
        returns[i].reserve(num_periods);
        
        for (int j = 0; j < num_periods; ++j) {
            returns[i].push_back(dist(gen));
        }
    }
    
    for (auto _ : state) {
        auto portfolio = stock_predict::PortfolioOptimizer::optimize_portfolio(returns, asset_names);
        benchmark::DoNotOptimize(portfolio);
    }
    
    state.SetComplexityN(state.range(0));
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_PortfolioOptimization)->Range(2, 50)->Complexity();

// Custom main to add additional benchmark configuration
int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    
    // Add custom benchmark configuration
    benchmark::AddCustomCounter("CPU_Cores", benchmark::Counter(std::thread::hardware_concurrency()));
    
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    
    return 0;
}
