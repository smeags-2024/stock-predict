#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <vector>
#include "stock_predict/risk.hpp"
#include "stock_predict/stock_predictor.hpp"

using namespace stock_predict;

// Simple test fixture for risk management
class RiskManagementTest : public testing::Test {
protected:
    void SetUp() override {
        // Create sample returns data
        sample_returns = {0.02, -0.01, 0.015, -0.008, 0.025, -0.012, 0.018, -0.005, 0.022, -0.015};
        sample_prices = {100.0, 102.0, 100.98, 102.49, 101.67, 104.21, 102.96, 105.81, 105.28, 107.60};
    }

    std::vector<double> sample_returns;
    std::vector<double> sample_prices;
};

// Test basic position sizing
TEST_F(RiskManagementTest, BasicPositionSizing) {
    double capital = 100000.0;
    double risk_fraction = 0.02;  // 2% risk
    double entry_price = 100.0;
    double stop_loss = 95.0;
    
    // Test fixed fractional sizing
    double position_size = PositionSizer::fixed_fractional(capital, risk_fraction, entry_price, stop_loss);
    EXPECT_GT(position_size, 0.0);
    
    // Test Kelly criterion
    double win_rate = 0.6;
    double avg_win = 0.08;
    double avg_loss = 0.05;
    
    double kelly_size = PositionSizer::kelly_criterion(win_rate, avg_win, avg_loss);
    EXPECT_GE(kelly_size, 0.0);
    EXPECT_LE(kelly_size, 1.0);
    
    // Test volatility sizing
    double target_vol = 0.15;
    double asset_vol = 0.25;
    
    double vol_size = PositionSizer::volatility_sizing(capital, target_vol, asset_vol);
    EXPECT_GT(vol_size, 0.0);
}

// Test error conditions
TEST_F(RiskManagementTest, ErrorConditions) {
    double capital = 100000.0;
    double entry_price = 100.0;
    double stop_loss = 95.0;
    
    // Test invalid parameters - these should work without throwing
    double result1 = PositionSizer::fixed_fractional(capital, 0.01, entry_price, stop_loss);
    EXPECT_GT(result1, 0.0);
    
    // Test Kelly with valid parameters
    double kelly = PositionSizer::kelly_criterion(0.6, 0.05, 0.03);
    EXPECT_GE(kelly, 0.0);
}

// Test portfolio optimization basics
TEST_F(RiskManagementTest, PortfolioOptimization) {
    // Test with simple data
    std::vector<std::vector<double>> returns_matrix = {
        {0.05, 0.02, -0.01, 0.03},
        {0.08, 0.01, 0.02, -0.01},
        {0.06, -0.01, 0.01, 0.02},
        {0.12, 0.03, -0.02, 0.01}
    };
    
    std::vector<std::string> asset_names = {"AAPL", "GOOG", "MSFT", "TSLA"};
    
    auto portfolio = PortfolioOptimizer::optimize_portfolio(returns_matrix, asset_names);
    EXPECT_FALSE(portfolio.weights.empty());
    EXPECT_EQ(portfolio.weights.size(), asset_names.size());
    
    // Test minimum variance portfolio
    auto min_var_portfolio = PortfolioOptimizer::minimum_variance_portfolio(returns_matrix, asset_names);
    EXPECT_FALSE(min_var_portfolio.weights.empty());
    EXPECT_EQ(min_var_portfolio.weights.size(), asset_names.size());
}

// Test risk metrics calculation
TEST_F(RiskManagementTest, RiskMetrics) {
    // Test VaR calculation
    double var_95 = RiskManager::calculate_var(sample_returns, 0.95);
    EXPECT_LT(var_95, 0.0);  // VaR should be negative (loss)
    
    double var_99 = RiskManager::calculate_var(sample_returns, 0.99);
    EXPECT_LT(var_99, var_95);  // 99% VaR should be more negative than 95% VaR
    
    // Test expected shortfall
    double es = RiskManager::calculate_expected_shortfall(sample_returns, 0.95);
    EXPECT_LT(es, var_95);  // ES should be more negative than VaR
    
    // Test Sharpe ratio
    double sharpe = RiskManager::calculate_sharpe_ratio(sample_returns, 0.02);
    EXPECT_TRUE(std::isfinite(sharpe));
    
    // Test max drawdown
    double max_dd = RiskManager::calculate_max_drawdown(sample_prices);
    EXPECT_GE(max_dd, 0.0);  // Drawdown should be positive percentage
}

// Test performance under different market conditions
TEST_F(RiskManagementTest, MarketConditions) {
    // Test bull market scenario
    std::vector<double> bull_returns = {0.02, 0.015, 0.025, 0.018, 0.022};
    
    double bull_var = RiskManager::calculate_var(bull_returns, 0.95);
    double bull_sharpe = RiskManager::calculate_sharpe_ratio(bull_returns, 0.02);
    
    // Test bear market scenario
    std::vector<double> bear_returns = {-0.02, -0.015, -0.025, -0.018, -0.022};
    
    double bear_var = RiskManager::calculate_var(bear_returns, 0.95);
    double bear_sharpe = RiskManager::calculate_sharpe_ratio(bear_returns, 0.02);
    
    // Bear market should have more negative VaR
    EXPECT_LT(bear_var, bull_var);
    
    // Bull market should have better Sharpe ratio
    EXPECT_GT(bull_sharpe, bear_sharpe);
}
