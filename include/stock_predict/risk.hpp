#pragma once

#include "stock_predictor.hpp"
#include <vector>
#include <string>

namespace stock_predict {

/**
 * @brief Risk calculation and management utilities
 */
class RiskManager {
public:
    /**
     * @brief Calculate Value at Risk (VaR)
     * @param returns Historical returns
     * @param confidence_level Confidence level (e.g., 0.95 for 95% VaR)
     * @param method Method to use ("historical", "parametric", "monte_carlo")
     * @return VaR value
     */
    static double calculate_var(const std::vector<double>& returns,
                               double confidence_level = 0.95,
                               const std::string& method = "historical");
    
    /**
     * @brief Calculate Expected Shortfall (Conditional VaR)
     * @param returns Historical returns
     * @param confidence_level Confidence level
     * @return Expected shortfall
     */
    static double calculate_expected_shortfall(const std::vector<double>& returns,
                                              double confidence_level = 0.95);
    
    /**
     * @brief Calculate Sharpe ratio
     * @param returns Portfolio returns
     * @param risk_free_rate Risk-free rate (annualized)
     * @return Sharpe ratio
     */
    static double calculate_sharpe_ratio(const std::vector<double>& returns,
                                        double risk_free_rate = 0.02);
    
    /**
     * @brief Calculate maximum drawdown
     * @param prices Price series
     * @return Maximum drawdown as percentage
     */
    static double calculate_max_drawdown(const std::vector<double>& prices);
    
    /**
     * @brief Calculate Calmar ratio (return/max drawdown)
     * @param returns Portfolio returns
     * @return Calmar ratio
     */
    static double calculate_calmar_ratio(const std::vector<double>& returns);
    
    /**
     * @brief Calculate volatility (annualized)
     * @param returns Daily returns
     * @param trading_days Number of trading days per year (default 252)
     * @return Annualized volatility
     */
    static double calculate_volatility(const std::vector<double>& returns,
                                      int trading_days = 252);
    
    /**
     * @brief Calculate beta relative to market
     * @param asset_returns Asset returns
     * @param market_returns Market returns
     * @return Beta coefficient
     */
    static double calculate_beta(const std::vector<double>& asset_returns,
                                const std::vector<double>& market_returns);
    
    /**
     * @brief Calculate tracking error
     * @param portfolio_returns Portfolio returns
     * @param benchmark_returns Benchmark returns
     * @return Tracking error (annualized)
     */
    static double calculate_tracking_error(const std::vector<double>& portfolio_returns,
                                          const std::vector<double>& benchmark_returns);
    
    /**
     * @brief Calculate information ratio
     * @param portfolio_returns Portfolio returns
     * @param benchmark_returns Benchmark returns
     * @return Information ratio
     */
    static double calculate_information_ratio(const std::vector<double>& portfolio_returns,
                                             const std::vector<double>& benchmark_returns);

private:
    static double percentile(std::vector<double> data, double p);
    static double mean(const std::vector<double>& data);
    static double standard_deviation(const std::vector<double>& data);
    static double covariance(const std::vector<double>& x, const std::vector<double>& y);
};

/**
 * @brief Portfolio optimization using Modern Portfolio Theory
 */
class PortfolioOptimizer {
public:
    /**
     * @brief Portfolio allocation result
     */
    struct Portfolio {
        std::vector<std::string> assets;
        std::vector<double> weights;
        double expected_return;
        double volatility;
        double sharpe_ratio;
    };

    /**
     * @brief Find optimal portfolio weights using mean-variance optimization
     * @param returns Matrix of asset returns (assets x time)
     * @param asset_names Names of assets
     * @param risk_free_rate Risk-free rate
     * @param target_return Target return (optional, for efficient frontier)
     * @return Optimal portfolio
     */
    static Portfolio optimize_portfolio(const std::vector<std::vector<double>>& returns,
                                       const std::vector<std::string>& asset_names,
                                       double risk_free_rate = 0.02,
                                       std::optional<double> target_return = std::nullopt);
    
    /**
     * @brief Calculate efficient frontier
     * @param returns Matrix of asset returns
     * @param asset_names Names of assets
     * @param num_points Number of points on frontier
     * @return Vector of portfolios on efficient frontier
     */
    static std::vector<Portfolio> efficient_frontier(const std::vector<std::vector<double>>& returns,
                                                    const std::vector<std::string>& asset_names,
                                                    int num_points = 50);
    
    /**
     * @brief Calculate risk parity portfolio
     * @param returns Matrix of asset returns
     * @param asset_names Names of assets
     * @return Risk parity portfolio
     */
    static Portfolio risk_parity_portfolio(const std::vector<std::vector<double>>& returns,
                                          const std::vector<std::string>& asset_names);
    
    /**
     * @brief Calculate minimum variance portfolio
     * @param returns Matrix of asset returns
     * @param asset_names Names of assets
     * @return Minimum variance portfolio
     */
    static Portfolio minimum_variance_portfolio(const std::vector<std::vector<double>>& returns,
                                               const std::vector<std::string>& asset_names);

private:
    static std::vector<std::vector<double>> calculate_covariance_matrix(
        const std::vector<std::vector<double>>& returns);
    static std::vector<double> calculate_expected_returns(
        const std::vector<std::vector<double>>& returns);
};

/**
 * @brief Position sizing and risk management
 */
class PositionSizer {
public:
    /**
     * @brief Kelly criterion for optimal bet sizing
     * @param win_rate Probability of winning trade
     * @param avg_win Average winning amount
     * @param avg_loss Average losing amount
     * @return Optimal fraction of capital to risk
     */
    static double kelly_criterion(double win_rate, double avg_win, double avg_loss);
    
    /**
     * @brief Fixed fractional position sizing
     * @param capital Total capital
     * @param risk_fraction Fraction of capital to risk per trade
     * @param entry_price Entry price
     * @param stop_loss Stop loss price
     * @return Position size
     */
    static double fixed_fractional(double capital, double risk_fraction,
                                  double entry_price, double stop_loss);
    
    /**
     * @brief Volatility-based position sizing
     * @param capital Total capital
     * @param target_volatility Target portfolio volatility
     * @param asset_volatility Asset volatility
     * @return Position size as fraction of capital
     */
    static double volatility_sizing(double capital, double target_volatility,
                                   double asset_volatility);
    
    /**
     * @brief Calculate optimal position size based on VaR constraint
     * @param capital Total capital
     * @param max_var Maximum acceptable VaR
     * @param asset_var Asset VaR
     * @return Position size
     */
    static double var_sizing(double capital, double max_var, double asset_var);
};

} // namespace stock_predict
