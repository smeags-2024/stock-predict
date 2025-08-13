#include <gtest/gtest.h>
#include "stock_predict/risk.hpp"
#include <vector>
#include <random>
#include <cmath>

class RiskManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Generate sample returns data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dist(0.001, 0.02); // 0.1% mean return, 2% volatility
        
        for (int i = 0; i < 252; ++i) { // 1 year of daily returns
            sample_returns_.push_back(dist(gen));
        }
        
        // Generate sample prices
        double price = 100.0;
        sample_prices_.push_back(price);
        for (double ret : sample_returns_) {
            price *= (1.0 + ret);
            sample_prices_.push_back(price);
        }
    }

    std::vector<double> sample_returns_;
    std::vector<double> sample_prices_;
};

TEST_F(RiskManagerTest, VaRCalculationHistorical) {
    auto var_95 = stock_predict::RiskManager::calculate_var(sample_returns_, 0.95, "historical");
    auto var_99 = stock_predict::RiskManager::calculate_var(sample_returns_, 0.99, "historical");
    
    // VaR should be negative (representing loss)
    EXPECT_LT(var_95, 0.0);
    EXPECT_LT(var_99, 0.0);
    
    // 99% VaR should be more extreme than 95% VaR
    EXPECT_LT(var_99, var_95);
    
    // VaR should be reasonable for our sample data (between -10% and 0%)
    EXPECT_GT(var_95, -0.1);
    EXPECT_LT(var_95, 0.0);
}

TEST_F(RiskManagerTest, VaRCalculationParametric) {
    auto var_95 = stock_predict::RiskManager::calculate_var(sample_returns_, 0.95, "parametric");
    
    EXPECT_LT(var_95, 0.0);
    EXPECT_GT(var_95, -0.1);
}

TEST_F(RiskManagerTest, ExpectedShortfall) {
    auto es = stock_predict::RiskManager::calculate_expected_shortfall(sample_returns_, 0.95);
    auto var = stock_predict::RiskManager::calculate_var(sample_returns_, 0.95);
    
    // Expected shortfall should be more extreme than VaR
    EXPECT_LT(es, var);
    EXPECT_LT(es, 0.0);
}

TEST_F(RiskManagerTest, SharpeRatio) {
    auto sharpe = stock_predict::RiskManager::calculate_sharpe_ratio(sample_returns_, 0.02);
    
    // Sharpe ratio should be finite
    EXPECT_TRUE(std::isfinite(sharpe));
    
    // For our sample data with positive mean return, Sharpe should be positive
    // (though it might be negative due to randomness)
    EXPECT_GT(sharpe, -5.0); // Reasonable lower bound
    EXPECT_LT(sharpe, 5.0);  // Reasonable upper bound
}

TEST_F(RiskManagerTest, MaxDrawdown) {
    auto max_dd = stock_predict::RiskManager::calculate_max_drawdown(sample_prices_);
    
    // Max drawdown should be non-positive
    EXPECT_LE(max_dd, 0.0);
    
    // Should be reasonable for random walk data
    EXPECT_GT(max_dd, -0.5); // Not more than 50% drawdown for random data
}

TEST_F(RiskManagerTest, Volatility) {
    auto vol = stock_predict::RiskManager::calculate_volatility(sample_returns_);
    
    // Volatility should be positive
    EXPECT_GT(vol, 0.0);
    
    // Should be reasonable (between 1% and 100% annualized)
    EXPECT_GT(vol, 0.01);
    EXPECT_LT(vol, 1.0);
}

TEST_F(RiskManagerTest, Beta) {
    // Create correlated market returns
    std::vector<double> market_returns;
    for (size_t i = 0; i < sample_returns_.size(); ++i) {
        // Asset returns = 0.8 * market returns + noise
        double market_ret = sample_returns_[i] / 0.8;
        market_returns.push_back(market_ret);
    }
    
    auto beta = stock_predict::RiskManager::calculate_beta(sample_returns_, market_returns);
    
    // Beta should be finite and reasonable
    EXPECT_TRUE(std::isfinite(beta));
    EXPECT_GT(beta, 0.0);  // Should be positive for correlated assets
    EXPECT_LT(beta, 3.0);  // Reasonable upper bound
}

TEST_F(RiskManagerTest, TrackingError) {
    // Create benchmark returns (slightly different from asset returns)
    std::vector<double> benchmark_returns;
    for (double ret : sample_returns_) {
        benchmark_returns.push_back(ret + 0.001); // Small difference
    }
    
    auto te = stock_predict::RiskManager::calculate_tracking_error(sample_returns_, benchmark_returns);
    
    // Tracking error should be positive and small
    EXPECT_GT(te, 0.0);
    EXPECT_LT(te, 0.1); // Less than 10% annualized for similar returns
}

TEST_F(RiskManagerTest, InformationRatio) {
    std::vector<double> benchmark_returns;
    for (double ret : sample_returns_) {
        benchmark_returns.push_back(ret - 0.001); // Portfolio slightly outperforms
    }
    
    auto ir = stock_predict::RiskManager::calculate_information_ratio(sample_returns_, benchmark_returns);
    
    // Information ratio should be finite
    EXPECT_TRUE(std::isfinite(ir));
    
    // Should be positive since portfolio outperforms benchmark
    EXPECT_GT(ir, 0.0);
}

// Edge cases
TEST_F(RiskManagerTest, EmptyReturns) {
    std::vector<double> empty_returns;
    
    // Should handle empty data gracefully
    EXPECT_THROW(
        stock_predict::RiskManager::calculate_var(empty_returns, 0.95),
        std::exception
    );
}

TEST_F(RiskManagerTest, SingleReturn) {
    std::vector<double> single_return = {0.01};
    
    // Should handle single data point
    auto var = stock_predict::RiskManager::calculate_var(single_return, 0.95);
    EXPECT_TRUE(std::isfinite(var));
}

TEST_F(RiskManagerTest, ConstantReturns) {
    std::vector<double> constant_returns(100, 0.01); // All returns are 1%
    
    auto vol = stock_predict::RiskManager::calculate_volatility(constant_returns);
    
    // Volatility of constant returns should be very small
    EXPECT_LT(vol, 0.001);
}

// Position sizing tests
class PositionSizerTest : public ::testing::Test {};

TEST_F(PositionSizerTest, KellyCriterion) {
    // Favorable scenario: 60% win rate, 2% avg win, 1% avg loss
    auto kelly = stock_predict::PositionSizer::kelly_criterion(0.6, 0.02, 0.01);
    
    // Kelly should be positive for favorable odds
    EXPECT_GT(kelly, 0.0);
    EXPECT_LT(kelly, 1.0); // Should not recommend more than 100%
    
    // Unfavorable scenario: 40% win rate
    auto kelly_unfavorable = stock_predict::PositionSizer::kelly_criterion(0.4, 0.02, 0.01);
    
    // Kelly should be negative or zero for unfavorable odds
    EXPECT_LE(kelly_unfavorable, 0.0);
}

TEST_F(PositionSizerTest, FixedFractional) {
    double capital = 100000.0;
    double risk_fraction = 0.02; // 2% risk
    double entry_price = 100.0;
    double stop_loss = 95.0; // 5% stop loss
    
    auto position_size = stock_predict::PositionSizer::fixed_fractional(
        capital, risk_fraction, entry_price, stop_loss);
    
    // Position size should be positive
    EXPECT_GT(position_size, 0.0);
    
    // Risk should equal 2% of capital
    double actual_risk = position_size * (entry_price - stop_loss);
    double expected_risk = capital * risk_fraction;
    EXPECT_NEAR(actual_risk, expected_risk, 1.0); // Within $1
}

TEST_F(PositionSizerTest, VolatilitySizing) {
    double capital = 100000.0;
    double target_vol = 0.15; // 15% target volatility
    double asset_vol = 0.30;  // 30% asset volatility
    
    auto position_fraction = stock_predict::PositionSizer::volatility_sizing(
        capital, target_vol, asset_vol);
    
    // Should size down for high volatility asset
    EXPECT_GT(position_fraction, 0.0);
    EXPECT_LT(position_fraction, 1.0);
    EXPECT_NEAR(position_fraction, target_vol / asset_vol, 0.01);
}

TEST_F(PositionSizerTest, VaRSizing) {
    double capital = 100000.0;
    double max_var = 0.05;    // 5% max VaR
    double asset_var = 0.03;  // 3% asset VaR
    
    auto position_size = stock_predict::PositionSizer::var_sizing(capital, max_var, asset_var);
    
    EXPECT_GT(position_size, 0.0);
    EXPECT_LT(position_size, capital);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
