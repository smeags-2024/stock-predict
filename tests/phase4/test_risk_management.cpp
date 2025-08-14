#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <random>
#include <vector>

// Include risk management components
#include "stock_predict/data.hpp"
#include "stock_predict/risk.hpp"

class Phase4RiskManagement : public ::testing::Test {
   protected:
    void SetUp() override {
        initial_capital_ = 100000.0;
        
        // Generate sample portfolio and positions
        generateSampleData();
    }

    void TearDown() override {
        sample_data_.clear();
        sample_returns_.clear();
    }

    void generateSampleData() {
        // Generate sample market data for risk calculations
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> returns_dist(0.001, 0.02);

        // Generate returns data
        for (int i = 0; i < 252; ++i) {
            sample_returns_.push_back(returns_dist(gen));
        }

        // Generate price data
        double price = 100.0;
        auto start_time = std::chrono::system_clock::now() - std::chrono::hours(24 * 252);

        for (int i = 0; i < 252; ++i) {
            stock_predict::MarketData data;
            data.symbol = "TEST";
            data.timestamp = start_time + std::chrono::hours(24 * i);
            
            price *= (1.0 + sample_returns_[i]);
            data.open = price;
            data.high = price * 1.02;
            data.low = price * 0.98;
            data.close = price;
            data.volume = 1000000;
            data.adjusted_close = price;

            sample_data_.push_back(data);
        }
    }

    double initial_capital_;
    std::vector<stock_predict::MarketData> sample_data_;
    std::vector<double> sample_returns_;
};
                position.avg_cost * (0.95 + (i % 3) * 0.05);  // Some gains/losses
            position.market_value = position.shares * position.current_price;
            position.unrealized_pnl = position.market_value - (position.shares * position.avg_cost);
            position.weight = weights[i];

            sample_portfolio.push_back(position);
        }
    }

    void generateSamplePositions() {
        sample_positions.clear();

        for (const auto& portfolio_pos : sample_portfolio) {
            Position position;
            position.symbol = portfolio_pos.symbol;
            position.size = portfolio_pos.shares;
            position.entry_price = portfolio_pos.avg_cost;
            position.current_price = portfolio_pos.current_price;
            position.unrealized_pnl = portfolio_pos.unrealized_pnl;
            position.entry_time =
                std::chrono::system_clock::now() - std::chrono::hours(24 * (rand() % 30));
            position.position_type =
                (position.unrealized_pnl >= 0) ? PositionType::LONG : PositionType::SHORT;

            sample_positions.push_back(position);
        }
    }

    // Test data
    double initial_capital = 100000.0;
    std::vector<PortfolioPosition> sample_portfolio;
    std::vector<Position> sample_positions;
    std::vector<StockData> market_data;

    // Risk management components
    std::unique_ptr<RiskCalculator> risk_calculator;
    std::unique_ptr<PortfolioManager> portfolio_manager;
    std::unique_ptr<PositionSizer> position_sizer;
    std::unique_ptr<StopLossManager> stop_loss_manager;
};

// Test T4.1: Risk Calculation
class RiskCalculationTest : public Phase4RiskManagement {};

TEST_F(RiskCalculationTest, T4_1_1_ValueAtRisk) {
    // Test Value at Risk (VaR) calculation
    std::vector<double> returns;

    // Generate historical returns for VaR calculation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> return_dist(0.001, 0.02);  // 0.1% mean return, 2% volatility

    for (int i = 0; i < 252; ++i) {  // 1 year of daily returns
        returns.push_back(return_dist(gen));
    }

    // Test different confidence levels
    double var_95 = risk_calculator->calculateVaR(returns, 0.95);
    double var_99 = risk_calculator->calculateVaR(returns, 0.99);

    EXPECT_LT(var_95, 0.0);                         // VaR should be negative (loss)
    EXPECT_LT(var_99, var_95);                      // 99% VaR should be more negative than 95% VaR
    EXPECT_GT(std::abs(var_99), std::abs(var_95));  // 99% VaR should have larger magnitude

    // Test parametric VaR
    double parametric_var = risk_calculator->calculateParametricVaR(returns, 0.95);
    EXPECT_LT(parametric_var, 0.0);

    // Test historical VaR
    double historical_var = risk_calculator->calculateHistoricalVaR(returns, 0.95);
    EXPECT_LT(historical_var, 0.0);

    // Test edge cases
    EXPECT_THROW(risk_calculator->calculateVaR(returns, 0.0), std::invalid_argument);
    EXPECT_THROW(risk_calculator->calculateVaR(returns, 1.0), std::invalid_argument);
    EXPECT_THROW(risk_calculator->calculateVaR({}, 0.95), std::invalid_argument);
}

TEST_F(RiskCalculationTest, T4_1_2_ConditionalValueAtRisk) {
    std::vector<double> returns;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> return_dist(0.001, 0.02);

    for (int i = 0; i < 252; ++i) {
        returns.push_back(return_dist(gen));
    }

    // Test Conditional Value at Risk (Expected Shortfall)
    double cvar_95 = risk_calculator->calculateCVaR(returns, 0.95);
    double var_95 = risk_calculator->calculateVaR(returns, 0.95);

    EXPECT_LT(cvar_95, 0.0);     // CVaR should be negative
    EXPECT_LT(cvar_95, var_95);  // CVaR should be worse than VaR

    // Test different confidence levels
    double cvar_99 = risk_calculator->calculateCVaR(returns, 0.99);
    EXPECT_LT(cvar_99, cvar_95);  // Higher confidence should give worse CVaR
}

TEST_F(RiskCalculationTest, T4_1_3_VolatilityMeasures) {
    std::vector<double> prices;

    // Generate price series
    double price = 100.0;
    for (int i = 0; i < 252; ++i) {
        prices.push_back(price);
        price *= (1.0 + ((rand() % 201 - 100) / 10000.0));  // Random walk with ±1% daily changes
    }

    // Test historical volatility
    double hist_vol = risk_calculator->calculateHistoricalVolatility(prices, 30);
    EXPECT_GT(hist_vol, 0.0);
    EXPECT_LT(hist_vol, 1.0);  // Reasonable volatility range

    // Test GARCH volatility
    double garch_vol = risk_calculator->calculateGARCHVolatility(prices);
    EXPECT_GT(garch_vol, 0.0);

    // Test EWMA volatility
    double ewma_vol = risk_calculator->calculateEWMAVolatility(prices, 0.94);
    EXPECT_GT(ewma_vol, 0.0);

    // Test Parkinson volatility (uses OHLC data)
    std::vector<StockData> ohlc_data;
    for (size_t i = 0; i < prices.size() - 1; ++i) {
        StockData data;
        data.symbol = "TEST";
        data.close = prices[i];
        data.open = prices[i] * (0.995 + (rand() % 11 - 5) / 1000.0);
        data.high = std::max(data.open, data.close) * (1.0 + (rand() % 21) / 10000.0);
        data.low = std::min(data.open, data.close) * (1.0 - (rand() % 21) / 10000.0);
        data.volume = 1000000;
        ohlc_data.push_back(data);
    }

    double parkinson_vol = risk_calculator->calculateParkinsonVolatility(ohlc_data);
    EXPECT_GT(parkinson_vol, 0.0);
}

TEST_F(RiskCalculationTest, T4_1_4_CorrelationRisk) {
    // Test correlation matrix calculation
    std::vector<std::vector<double>> returns_matrix;

    // Generate correlated returns for multiple assets
    for (int asset = 0; asset < 5; ++asset) {
        std::vector<double> asset_returns;
        for (int day = 0; day < 100; ++day) {
            double return_val =
                ((rand() % 201 - 100) / 10000.0) + asset * 0.0001;  // Slight bias per asset
            asset_returns.push_back(return_val);
        }
        returns_matrix.push_back(asset_returns);
    }

    auto correlation_matrix = risk_calculator->calculateCorrelationMatrix(returns_matrix);

    // Correlation matrix should be square
    EXPECT_EQ(correlation_matrix.size(), returns_matrix.size());
    for (const auto& row : correlation_matrix) {
        EXPECT_EQ(row.size(), returns_matrix.size());
    }

    // Diagonal elements should be 1.0
    for (size_t i = 0; i < correlation_matrix.size(); ++i) {
        EXPECT_NEAR(correlation_matrix[i][i], 1.0, 0.01);
    }

    // Matrix should be symmetric
    for (size_t i = 0; i < correlation_matrix.size(); ++i) {
        for (size_t j = 0; j < correlation_matrix.size(); ++j) {
            EXPECT_NEAR(correlation_matrix[i][j], correlation_matrix[j][i], 0.01);
        }
    }

    // Test portfolio correlation risk
    std::vector<double> weights = {0.2, 0.2, 0.2, 0.2, 0.2};
    double portfolio_var = risk_calculator->calculatePortfolioVariance(returns_matrix, weights);
    EXPECT_GT(portfolio_var, 0.0);
}

TEST_F(RiskCalculationTest, T4_1_5_DrawdownAnalysis) {
    std::vector<double> portfolio_values;

    // Generate portfolio value series with drawdowns
    double value = 100000.0;
    for (int i = 0; i < 252; ++i) {
        portfolio_values.push_back(value);

        // Simulate market movements with occasional drawdowns
        double change_pct = ((rand() % 201 - 100) / 10000.0);
        if (i > 100 && i < 120) {  // Simulate a drawdown period
            change_pct -= 0.01;    // Additional 1% daily loss
        }
        value *= (1.0 + change_pct);
    }

    // Test maximum drawdown
    double max_drawdown = risk_calculator->calculateMaxDrawdown(portfolio_values);
    EXPECT_LE(max_drawdown, 0.0);   // Drawdown should be negative or zero
    EXPECT_GT(max_drawdown, -1.0);  // Shouldn't lose more than 100%

    // Test drawdown duration
    auto drawdown_periods = risk_calculator->calculateDrawdownPeriods(portfolio_values);
    EXPECT_FALSE(drawdown_periods.empty());

    // Test recovery analysis
    auto recovery_times = risk_calculator->calculateRecoveryTimes(portfolio_values);
    for (const auto& recovery_time : recovery_times) {
        EXPECT_GT(recovery_time, 0);
    }
}

// Test T4.2: Portfolio Management
class PortfolioManagementTest : public Phase4RiskManagement {};

TEST_F(PortfolioManagementTest, T4_2_1_PortfolioConstruction) {
    // Test portfolio initialization
    EXPECT_DOUBLE_EQ(portfolio_manager->getTotalValue(), initial_capital);
    EXPECT_EQ(portfolio_manager->getPositionCount(), 0);

    // Test adding positions
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    EXPECT_EQ(portfolio_manager->getPositionCount(), sample_portfolio.size());
    EXPECT_GT(portfolio_manager->getTotalValue(), 0.0);

    // Test portfolio weights
    auto weights = portfolio_manager->getPositionWeights();
    double total_weight = 0.0;
    for (const auto& weight : weights) {
        total_weight += weight.second;
        EXPECT_GE(weight.second, 0.0);
        EXPECT_LE(weight.second, 1.0);
    }
    EXPECT_NEAR(total_weight, 1.0, 0.01);  // Weights should sum to 1
}

TEST_F(PortfolioManagementTest, T4_2_2_PortfolioRebalancing) {
    // Add positions to portfolio
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    // Define target allocation
    std::map<std::string, double> target_allocation;
    for (const auto& position : sample_portfolio) {
        target_allocation[position.symbol] = 0.10;  // Equal weight
    }

    // Test rebalancing
    auto rebalancing_trades = portfolio_manager->calculateRebalancing(target_allocation);
    EXPECT_FALSE(rebalancing_trades.empty());

    // Execute rebalancing
    portfolio_manager->executeRebalancing(rebalancing_trades);

    // Verify weights are closer to target after rebalancing
    auto new_weights = portfolio_manager->getPositionWeights();
    for (const auto& weight : new_weights) {
        EXPECT_NEAR(weight.second, 0.10, 0.02);  // Within 2% of target
    }
}

TEST_F(PortfolioManagementTest, T4_2_3_PerformanceMetrics) {
    // Add positions and simulate some performance
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    // Test basic performance metrics
    double total_return = portfolio_manager->getTotalReturn();
    EXPECT_NE(total_return, 0.0);  // Should have some return

    double daily_return = portfolio_manager->getDailyReturn();
    EXPECT_GT(daily_return, -1.0);  // Shouldn't lose more than 100% in a day
    EXPECT_LT(daily_return, 1.0);   // Shouldn't gain more than 100% in a day

    // Test Sharpe ratio calculation
    std::vector<double> daily_returns;
    for (int i = 0; i < 30; ++i) {
        daily_returns.push_back(((rand() % 201 - 100) / 10000.0));
    }

    double sharpe_ratio =
        portfolio_manager->calculateSharpeRatio(daily_returns, 0.02);  // 2% risk-free rate
    EXPECT_GT(sharpe_ratio, -5.0);                                     // Reasonable range
    EXPECT_LT(sharpe_ratio, 5.0);

    // Test Sortino ratio
    double sortino_ratio = portfolio_manager->calculateSortinoRatio(daily_returns, 0.02);
    EXPECT_GE(sortino_ratio, sharpe_ratio);  // Sortino should be >= Sharpe

    // Test maximum drawdown
    double max_dd = portfolio_manager->getMaxDrawdown();
    EXPECT_LE(max_dd, 0.0);
}

TEST_F(PortfolioManagementTest, T4_2_4_RiskBudgeting) {
    // Add positions to portfolio
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    // Test risk contribution calculation
    auto risk_contributions = portfolio_manager->calculateRiskContributions();
    EXPECT_FALSE(risk_contributions.empty());

    double total_risk_contribution = 0.0;
    for (const auto& contribution : risk_contributions) {
        EXPECT_GE(contribution.second, 0.0);
        total_risk_contribution += contribution.second;
    }
    EXPECT_NEAR(total_risk_contribution, 1.0, 0.01);  // Should sum to 1

    // Test risk budgeting optimization
    std::map<std::string, double> risk_budget;
    for (const auto& position : sample_portfolio) {
        risk_budget[position.symbol] = 0.10;  // Equal risk budget
    }

    auto risk_budgeted_weights = portfolio_manager->optimizeRiskBudget(risk_budget);
    EXPECT_FALSE(risk_budgeted_weights.empty());

    double total_weight = 0.0;
    for (const auto& weight : risk_budgeted_weights) {
        EXPECT_GT(weight.second, 0.0);
        total_weight += weight.second;
    }
    EXPECT_NEAR(total_weight, 1.0, 0.01);
}

TEST_F(PortfolioManagementTest, T4_2_5_PortfolioOptimization) {
    // Generate return expectations and covariance matrix
    std::vector<double> expected_returns;
    std::vector<std::vector<double>> covariance_matrix;

    size_t n_assets = sample_portfolio.size();

    // Generate expected returns
    for (size_t i = 0; i < n_assets; ++i) {
        expected_returns.push_back(0.08 + (rand() % 101 - 50) / 10000.0);  // Around 8% with noise
    }

    // Generate covariance matrix
    for (size_t i = 0; i < n_assets; ++i) {
        std::vector<double> row;
        for (size_t j = 0; j < n_assets; ++j) {
            if (i == j) {
                row.push_back(0.04);  // 4% variance on diagonal
            } else {
                row.push_back(0.01);  // 1% covariance off-diagonal
            }
        }
        covariance_matrix.push_back(row);
    }

    // Test mean-variance optimization
    auto optimal_weights =
        portfolio_manager->optimizeMeanVariance(expected_returns, covariance_matrix, 0.10);
    EXPECT_EQ(optimal_weights.size(), n_assets);

    double total_weight = 0.0;
    for (double weight : optimal_weights) {
        EXPECT_GE(weight, 0.0);  // Long-only constraint
        EXPECT_LE(weight, 1.0);
        total_weight += weight;
    }
    EXPECT_NEAR(total_weight, 1.0, 0.01);

    // Test minimum variance optimization
    auto min_var_weights = portfolio_manager->optimizeMinimumVariance(covariance_matrix);
    EXPECT_EQ(min_var_weights.size(), n_assets);

    total_weight = 0.0;
    for (double weight : min_var_weights) {
        EXPECT_GE(weight, 0.0);
        total_weight += weight;
    }
    EXPECT_NEAR(total_weight, 1.0, 0.01);

    // Test maximum Sharpe ratio optimization
    auto max_sharpe_weights =
        portfolio_manager->optimizeMaxSharpe(expected_returns, covariance_matrix, 0.02);
    EXPECT_EQ(max_sharpe_weights.size(), n_assets);

    total_weight = 0.0;
    for (double weight : max_sharpe_weights) {
        EXPECT_GE(weight, 0.0);
        total_weight += weight;
    }
    EXPECT_NEAR(total_weight, 1.0, 0.01);
}

// Test T4.3: Position Sizing
class PositionSizingTest : public Phase4RiskManagement {};

TEST_F(PositionSizingTest, T4_3_1_FixedFractionalSizing) {
    double account_value = 100000.0;
    double risk_per_trade = 0.02;  // 2% risk per trade

    // Test fixed fractional position sizing
    double position_size = position_sizer->calculateFixedFractional(account_value, risk_per_trade);
    EXPECT_DOUBLE_EQ(position_size, account_value * risk_per_trade);

    // Test with different risk levels
    double conservative_size = position_sizer->calculateFixedFractional(account_value, 0.01);
    double aggressive_size = position_sizer->calculateFixedFractional(account_value, 0.05);

    EXPECT_LT(conservative_size, position_size);
    EXPECT_GT(aggressive_size, position_size);

    // Test edge cases
    EXPECT_THROW(position_sizer->calculateFixedFractional(account_value, 0.0),
                 std::invalid_argument);
    EXPECT_THROW(position_sizer->calculateFixedFractional(account_value, 1.1),
                 std::invalid_argument);
    EXPECT_THROW(position_sizer->calculateFixedFractional(-1000.0, 0.02), std::invalid_argument);
}

TEST_F(PositionSizingTest, T4_3_2_KellyPosition) {
    double win_rate = 0.55;   // 55% win rate
    double avg_win = 0.08;    // 8% average win
    double avg_loss = -0.05;  // 5% average loss

    // Test Kelly criterion position sizing
    double kelly_fraction =
        position_sizer->calculateKellyFraction(win_rate, avg_win, std::abs(avg_loss));
    EXPECT_GT(kelly_fraction, 0.0);
    EXPECT_LE(kelly_fraction, 1.0);

    // Test Kelly position size
    double account_value = 100000.0;
    double kelly_position = position_sizer->calculateKellyPosition(account_value, win_rate, avg_win,
                                                                   std::abs(avg_loss));
    EXPECT_GT(kelly_position, 0.0);
    EXPECT_LE(kelly_position, account_value);

    // Test fractional Kelly (more conservative)
    double fractional_kelly = position_sizer->calculateFractionalKelly(
        account_value, win_rate, avg_win, std::abs(avg_loss), 0.5);
    EXPECT_LT(fractional_kelly, kelly_position);
    EXPECT_GT(fractional_kelly, 0.0);

    // Test with losing system (negative edge)
    double losing_kelly = position_sizer->calculateKellyFraction(0.40, 0.05, 0.08);
    EXPECT_LE(losing_kelly, 0.0);  // Should recommend no position or short
}

TEST_F(PositionSizingTest, T4_3_3_VolatilityBasedSizing) {
    double account_value = 100000.0;
    double target_volatility = 0.15;  // 15% target portfolio volatility
    double asset_volatility = 0.25;   // 25% asset volatility

    // Test volatility-based position sizing
    double vol_position = position_sizer->calculateVolatilityBased(account_value, target_volatility,
                                                                   asset_volatility);
    EXPECT_GT(vol_position, 0.0);
    EXPECT_LT(vol_position, account_value);

    // Higher asset volatility should result in smaller position
    double high_vol_position =
        position_sizer->calculateVolatilityBased(account_value, target_volatility, 0.50);
    EXPECT_LT(high_vol_position, vol_position);

    // Test with correlation adjustment
    double correlation = 0.3;  // 30% correlation with existing portfolio
    double corr_adjusted_position = position_sizer->calculateVolatilityBasedWithCorrelation(
        account_value, target_volatility, asset_volatility, correlation);

    EXPECT_GT(corr_adjusted_position, 0.0);
    // Position should be smaller when there's positive correlation
    EXPECT_LT(corr_adjusted_position, vol_position);
}

TEST_F(PositionSizingTest, T4_3_4_RiskParitySize) {
    std::vector<double> asset_volatilities = {0.20, 0.25, 0.30, 0.15, 0.35};
    double total_capital = 100000.0;

    // Test equal risk contribution sizing
    auto risk_parity_sizes = position_sizer->calculateRiskParity(total_capital, asset_volatilities);
    EXPECT_EQ(risk_parity_sizes.size(), asset_volatilities.size());

    // Total should sum to total capital
    double total_allocated = 0.0;
    for (double size : risk_parity_sizes) {
        EXPECT_GT(size, 0.0);
        total_allocated += size;
    }
    EXPECT_NEAR(total_allocated, total_capital, 1.0);  // Within $1

    // Higher volatility assets should get smaller positions
    for (size_t i = 0; i < asset_volatilities.size(); ++i) {
        for (size_t j = i + 1; j < asset_volatilities.size(); ++j) {
            if (asset_volatilities[i] > asset_volatilities[j]) {
                EXPECT_LT(risk_parity_sizes[i], risk_parity_sizes[j]);
            }
        }
    }

    // Test risk parity with correlation matrix
    std::vector<std::vector<double>> correlation_matrix;
    size_t n = asset_volatilities.size();
    for (size_t i = 0; i < n; ++i) {
        std::vector<double> row;
        for (size_t j = 0; j < n; ++j) {
            if (i == j) {
                row.push_back(1.0);
            } else {
                row.push_back(0.3);  // 30% correlation
            }
        }
        correlation_matrix.push_back(row);
    }

    auto corr_risk_parity = position_sizer->calculateRiskParityWithCorrelation(
        total_capital, asset_volatilities, correlation_matrix);

    EXPECT_EQ(corr_risk_parity.size(), asset_volatilities.size());

    total_allocated = 0.0;
    for (double size : corr_risk_parity) {
        EXPECT_GT(size, 0.0);
        total_allocated += size;
    }
    EXPECT_NEAR(total_allocated, total_capital, 1.0);
}

TEST_F(PositionSizingTest, T4_3_5_AdaptivePositionSizing) {
    double account_value = 100000.0;

    // Test performance-based adaptive sizing
    std::vector<double> recent_returns = {0.02, -0.01, 0.03, -0.02, 0.01};  // Mixed performance
    double base_risk = 0.02;

    double adaptive_size =
        position_sizer->calculateAdaptiveSize(account_value, recent_returns, base_risk);
    EXPECT_GT(adaptive_size, 0.0);
    EXPECT_LE(adaptive_size, account_value);

    // Test with winning streak
    std::vector<double> winning_returns = {0.02, 0.03, 0.01, 0.04, 0.02};
    double winning_size =
        position_sizer->calculateAdaptiveSize(account_value, winning_returns, base_risk);
    EXPECT_GT(winning_size, adaptive_size);  // Should size up after wins

    // Test with losing streak
    std::vector<double> losing_returns = {-0.02, -0.03, -0.01, -0.04, -0.02};
    double losing_size =
        position_sizer->calculateAdaptiveSize(account_value, losing_returns, base_risk);
    EXPECT_LT(losing_size, adaptive_size);  // Should size down after losses

    // Test drawdown-based adjustment
    double current_drawdown = 0.10;  // 10% drawdown
    double max_drawdown = 0.20;      // 20% max allowed drawdown

    double drawdown_adjusted = position_sizer->calculateDrawdownAdjusted(
        account_value, base_risk, current_drawdown, max_drawdown);

    EXPECT_LT(drawdown_adjusted, account_value * base_risk);  // Should reduce size during drawdown
    EXPECT_GT(drawdown_adjusted, 0.0);
}

// Test T4.4: Stop Loss Management
class StopLossManagementTest : public Phase4RiskManagement {};

TEST_F(StopLossManagementTest, T4_4_1_FixedStopLoss) {
    double entry_price = 100.0;
    double stop_percentage = 0.05;  // 5% stop loss

    // Test fixed percentage stop loss for long position
    double long_stop =
        stop_loss_manager->calculateFixedStopLoss(entry_price, stop_percentage, PositionType::LONG);
    EXPECT_DOUBLE_EQ(long_stop, entry_price * (1.0 - stop_percentage));

    // Test fixed percentage stop loss for short position
    double short_stop = stop_loss_manager->calculateFixedStopLoss(entry_price, stop_percentage,
                                                                  PositionType::SHORT);
    EXPECT_DOUBLE_EQ(short_stop, entry_price * (1.0 + stop_percentage));

    // Test fixed dollar amount stop loss
    double dollar_amount = 5.0;
    double long_dollar_stop = stop_loss_manager->calculateFixedDollarStopLoss(
        entry_price, dollar_amount, PositionType::LONG);
    EXPECT_DOUBLE_EQ(long_dollar_stop, entry_price - dollar_amount);

    double short_dollar_stop = stop_loss_manager->calculateFixedDollarStopLoss(
        entry_price, dollar_amount, PositionType::SHORT);
    EXPECT_DOUBLE_EQ(short_dollar_stop, entry_price + dollar_amount);
}

TEST_F(StopLossManagementTest, T4_4_2_ATRStopLoss) {
    // Generate sample price data for ATR calculation
    std::vector<StockData> price_data;
    double price = 100.0;

    for (int i = 0; i < 20; ++i) {
        StockData data;
        data.symbol = "TEST";
        data.close = price;
        data.open = price * (0.995 + (rand() % 11 - 5) / 1000.0);
        data.high = std::max(data.open, data.close) * (1.0 + (rand() % 21) / 1000.0);
        data.low = std::min(data.open, data.close) * (1.0 - (rand() % 21) / 1000.0);
        data.volume = 1000000;

        price_data.push_back(data);
        price *= (1.0 + ((rand() % 21 - 10) / 1000.0));  // Random walk
    }

    double entry_price = price_data.back().close;
    double atr_multiplier = 2.0;

    // Test ATR-based stop loss
    double atr_stop_long = stop_loss_manager->calculateATRStopLoss(
        entry_price, price_data, atr_multiplier, PositionType::LONG);
    EXPECT_LT(atr_stop_long, entry_price);  // Stop should be below entry for long

    double atr_stop_short = stop_loss_manager->calculateATRStopLoss(
        entry_price, price_data, atr_multiplier, PositionType::SHORT);
    EXPECT_GT(atr_stop_short, entry_price);  // Stop should be above entry for short

    // Test different ATR periods
    double atr_stop_14 = stop_loss_manager->calculateATRStopLoss(
        entry_price, price_data, atr_multiplier, PositionType::LONG, 14);
    double atr_stop_5 = stop_loss_manager->calculateATRStopLoss(
        entry_price, price_data, atr_multiplier, PositionType::LONG, 5);

    // Different periods may give different results
    EXPECT_GT(atr_stop_14, 0.0);
    EXPECT_GT(atr_stop_5, 0.0);
}

TEST_F(StopLossManagementTest, T4_4_3_TrailingStopLoss) {
    double entry_price = 100.0;
    double trail_percentage = 0.05;  // 5% trailing stop

    // Test trailing stop initialization
    auto trailing_stop = stop_loss_manager->initializeTrailingStop(entry_price, trail_percentage,
                                                                   PositionType::LONG);
    EXPECT_DOUBLE_EQ(trailing_stop.stop_price, entry_price * (1.0 - trail_percentage));
    EXPECT_DOUBLE_EQ(trailing_stop.highest_price, entry_price);

    // Test trailing stop updates with favorable price movement
    double new_high_price = 105.0;
    stop_loss_manager->updateTrailingStop(trailing_stop, new_high_price);

    EXPECT_DOUBLE_EQ(trailing_stop.highest_price, new_high_price);
    EXPECT_DOUBLE_EQ(trailing_stop.stop_price, new_high_price * (1.0 - trail_percentage));

    // Test that stop doesn't move down with adverse price movement
    double lower_price = 103.0;
    double previous_stop = trailing_stop.stop_price;
    stop_loss_manager->updateTrailingStop(trailing_stop, lower_price);

    EXPECT_DOUBLE_EQ(trailing_stop.stop_price, previous_stop);      // Stop shouldn't move down
    EXPECT_DOUBLE_EQ(trailing_stop.highest_price, new_high_price);  // High should remain

    // Test stop trigger
    double trigger_price = trailing_stop.stop_price - 0.01;
    bool should_trigger =
        stop_loss_manager->shouldTriggerStop(trailing_stop, trigger_price, PositionType::LONG);
    EXPECT_TRUE(should_trigger);

    double safe_price = trailing_stop.stop_price + 0.01;
    bool should_not_trigger =
        stop_loss_manager->shouldTriggerStop(trailing_stop, safe_price, PositionType::LONG);
    EXPECT_FALSE(should_not_trigger);
}

TEST_F(StopLossManagementTest, T4_4_4_VolatilityStopLoss) {
    // Generate price data with known volatility
    std::vector<double> prices;
    double price = 100.0;

    for (int i = 0; i < 50; ++i) {
        prices.push_back(price);
        price *= (1.0 + ((rand() % 41 - 20) / 1000.0));  // ±2% daily moves
    }

    double entry_price = prices.back();
    double vol_multiplier = 2.0;

    // Test volatility-based stop loss
    double vol_stop_long = stop_loss_manager->calculateVolatilityStopLoss(
        entry_price, prices, vol_multiplier, PositionType::LONG);
    EXPECT_LT(vol_stop_long, entry_price);

    double vol_stop_short = stop_loss_manager->calculateVolatilityStopLoss(
        entry_price, prices, vol_multiplier, PositionType::SHORT);
    EXPECT_GT(vol_stop_short, entry_price);

    // Test different volatility calculations
    double ewma_stop = stop_loss_manager->calculateEWMAVolatilityStop(
        entry_price, prices, vol_multiplier, PositionType::LONG, 0.94);
    EXPECT_LT(ewma_stop, entry_price);

    double garch_stop = stop_loss_manager->calculateGARCHVolatilityStop(
        entry_price, prices, vol_multiplier, PositionType::LONG);
    EXPECT_LT(garch_stop, entry_price);
}

TEST_F(StopLossManagementTest, T4_4_5_TimeBasedStopLoss) {
    auto entry_time = std::chrono::system_clock::now();
    double entry_price = 100.0;

    // Test time-based stop loss
    auto time_duration = std::chrono::hours(24);  // 1 day time stop
    auto time_stop = stop_loss_manager->calculateTimeBasedStop(entry_time, time_duration);

    EXPECT_GT(time_stop, entry_time);
    EXPECT_EQ(time_stop, entry_time + time_duration);

    // Test if time stop should trigger
    auto current_time = entry_time + std::chrono::hours(25);  // 1 hour past stop time
    bool should_trigger_time = stop_loss_manager->shouldTriggerTimeStop(time_stop, current_time);
    EXPECT_TRUE(should_trigger_time);

    auto early_time = entry_time + std::chrono::hours(23);  // Before stop time
    bool should_not_trigger_time = stop_loss_manager->shouldTriggerTimeStop(time_stop, early_time);
    EXPECT_FALSE(should_not_trigger_time);

    // Test profit target with time component
    double profit_target = 110.0;  // 10% profit target
    double current_price = 108.0;  // Not yet at target

    bool should_trigger_profit = stop_loss_manager->shouldTriggerProfitTarget(
        profit_target, current_price, PositionType::LONG);
    EXPECT_FALSE(should_trigger_profit);

    current_price = 111.0;  // Above target
    should_trigger_profit = stop_loss_manager->shouldTriggerProfitTarget(
        profit_target, current_price, PositionType::LONG);
    EXPECT_TRUE(should_trigger_profit);
}

// Integration and Performance Tests
class Phase4IntegrationTest : public Phase4RiskManagement {};

TEST_F(Phase4IntegrationTest, T4_5_1_IntegratedRiskSystem) {
    // Test complete risk management system integration
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    // Calculate portfolio-level risk metrics
    double portfolio_var = portfolio_manager->getPortfolioVaR(0.95);
    EXPECT_LT(portfolio_var, 0.0);

    double portfolio_cvar = portfolio_manager->getPortfolioCVaR(0.95);
    EXPECT_LT(portfolio_cvar, portfolio_var);

    // Test position sizing for new trade
    double account_value = portfolio_manager->getTotalValue();
    double risk_per_trade = 0.02;

    double position_size = position_sizer->calculateFixedFractional(account_value, risk_per_trade);
    EXPECT_GT(position_size, 0.0);
    EXPECT_LE(position_size, account_value);

    // Test stop loss for the new position
    double entry_price = 150.0;
    double stop_price =
        stop_loss_manager->calculateFixedStopLoss(entry_price, 0.05, PositionType::LONG);
    EXPECT_LT(stop_price, entry_price);

    // Verify integrated risk budget
    double position_risk = (entry_price - stop_price) * (position_size / entry_price);
    double expected_risk = account_value * risk_per_trade;
    EXPECT_NEAR(position_risk, expected_risk, expected_risk * 0.01);  // Within 1%
}

TEST_F(Phase4IntegrationTest, T4_5_2_RealTimeRiskMonitoring) {
    // Add positions to portfolio
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    // Simulate real-time price updates
    std::vector<std::future<void>> futures;

    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            // Simulate price update
            std::string symbol = sample_portfolio[i % sample_portfolio.size()].symbol;
            double new_price = sample_portfolio[i % sample_portfolio.size()].current_price *
                               (0.95 + (rand() % 101) / 1000.0);  // ±5% price change

            portfolio_manager->updatePositionPrice(symbol, new_price);

            // Check risk limits
            double current_var = portfolio_manager->getPortfolioVaR(0.95);
            EXPECT_LT(std::abs(current_var), 0.5);  // Risk should be reasonable
        }));
    }

    // Wait for all updates to complete
    for (auto& future : futures) {
        future.get();
    }

    // Verify portfolio integrity after concurrent updates
    EXPECT_GT(portfolio_manager->getTotalValue(), 0.0);
    EXPECT_EQ(portfolio_manager->getPositionCount(), sample_portfolio.size());
}

TEST_F(Phase4IntegrationTest, T4_5_3_StressTestingScenarios) {
    // Add positions to portfolio
    for (const auto& position : sample_portfolio) {
        portfolio_manager->addPosition(position);
    }

    double initial_value = portfolio_manager->getTotalValue();

    // Stress Test 1: Market crash scenario (-20% across all positions)
    for (const auto& position : sample_portfolio) {
        double stress_price = position.current_price * 0.8;  // 20% decline
        portfolio_manager->updatePositionPrice(position.symbol, stress_price);
    }

    double crash_value = portfolio_manager->getTotalValue();
    double crash_loss = (crash_value - initial_value) / initial_value;

    EXPECT_LT(crash_loss, -0.15);  // Should lose more than 15%
    EXPECT_GT(crash_loss, -0.25);  // But less than 25% due to correlation

    // Stress Test 2: Sector rotation (some up, some down)
    for (size_t i = 0; i < sample_portfolio.size(); ++i) {
        double stress_price;
        if (i % 2 == 0) {
            stress_price = sample_portfolio[i].current_price * 1.1;  // +10%
        } else {
            stress_price = sample_portfolio[i].current_price * 0.9;  // -10%
        }
        portfolio_manager->updatePositionPrice(sample_portfolio[i].symbol, stress_price);
    }

    double rotation_value = portfolio_manager->getTotalValue();
    EXPECT_GT(rotation_value, 0.0);

    // Stress Test 3: Volatility spike
    std::vector<double> high_vol_returns;
    for (int i = 0; i < 20; ++i) {
        high_vol_returns.push_back(((rand() % 201 - 100) / 1000.0));  // ±10% daily returns
    }

    double stress_var = risk_calculator->calculateVaR(high_vol_returns, 0.95);
    EXPECT_LT(stress_var, -0.05);  // Should show high risk
}

TEST_F(Phase4IntegrationTest, T4_5_4_BacktestingRiskModels) {
    // Generate historical returns for backtesting
    std::vector<std::vector<double>> historical_returns;

    for (size_t asset = 0; asset < sample_portfolio.size(); ++asset) {
        std::vector<double> asset_returns;
        for (int day = 0; day < 252; ++day) {                      // 1 year of data
            double return_val = ((rand() % 201 - 100) / 10000.0);  // ±1% daily returns
            asset_returns.push_back(return_val);
        }
        historical_returns.push_back(asset_returns);
    }

    // Backtest VaR model accuracy
    int var_breaches = 0;
    int total_days = historical_returns[0].size() - 30;  // Need 30 days for rolling VaR

    for (int day = 30; day < historical_returns[0].size(); ++day) {
        // Calculate portfolio return for this day
        double portfolio_return = 0.0;
        for (size_t asset = 0; asset < historical_returns.size(); ++asset) {
            portfolio_return += historical_returns[asset][day] * sample_portfolio[asset].weight;
        }

        // Calculate VaR using previous 30 days
        std::vector<double> rolling_returns;
        for (int prev_day = day - 30; prev_day < day; ++prev_day) {
            double prev_return = 0.0;
            for (size_t asset = 0; asset < historical_returns.size(); ++asset) {
                prev_return += historical_returns[asset][prev_day] * sample_portfolio[asset].weight;
            }
            rolling_returns.push_back(prev_return);
        }

        double var_95 = risk_calculator->calculateVaR(rolling_returns, 0.95);

        // Check if actual return breached VaR
        if (portfolio_return < var_95) {
            var_breaches++;
        }
    }

    // VaR should be breached approximately 5% of the time (for 95% VaR)
    double breach_rate = static_cast<double>(var_breaches) / total_days;
    EXPECT_GT(breach_rate, 0.02);  // At least 2%
    EXPECT_LT(breach_rate, 0.08);  // At most 8%
}

TEST_F(Phase4IntegrationTest, T4_5_5_RiskSystemPerformance) {
    // Test performance of risk calculations with large portfolio
    std::vector<PortfolioPosition> large_portfolio;

    // Create portfolio with 100 positions
    for (int i = 0; i < 100; ++i) {
        PortfolioPosition position;
        position.symbol = "STOCK" + std::to_string(i);
        position.shares = 100 + (rand() % 901);     // 100-1000 shares
        position.avg_cost = 50.0 + (rand() % 501);  // $50-$550 per share
        position.current_price =
            position.avg_cost * (0.9 + (rand() % 201) / 1000.0);  // ±10% from cost
        position.market_value = position.shares * position.current_price;
        position.unrealized_pnl = position.market_value - (position.shares * position.avg_cost);
        position.weight = 0.01;  // Equal weight

        large_portfolio.push_back(position);
    }

    // Test performance of portfolio operations
    auto start_time = std::chrono::high_resolution_clock::now();

    // Add all positions
    for (const auto& position : large_portfolio) {
        portfolio_manager->addPosition(position);
    }

    // Calculate risk metrics
    double var = portfolio_manager->getPortfolioVaR(0.95);
    auto risk_contributions = portfolio_manager->calculateRiskContributions();
    auto weights = portfolio_manager->getPositionWeights();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Performance should be reasonable for large portfolio
    EXPECT_LT(duration.count(), 1000);  // Less than 1 second

    // Results should be valid
    EXPECT_LT(var, 0.0);
    EXPECT_EQ(risk_contributions.size(), large_portfolio.size());
    EXPECT_EQ(weights.size(), large_portfolio.size());

    // Test memory usage doesn't grow excessively
    EXPECT_EQ(portfolio_manager->getPositionCount(), large_portfolio.size());
}
