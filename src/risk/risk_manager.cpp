#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include "stock_predict/risk.hpp"

namespace stock_predict {

// Risk Manager Implementation

double RiskManager::calculate_var(const std::vector<double>& returns, double confidence_level,
                                  const std::string& method) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }

    if (method == "historical") {
        std::vector<double> sorted_returns = returns;
        std::sort(sorted_returns.begin(), sorted_returns.end());

        double percentile_pos = (1.0 - confidence_level) * 100.0;
        return percentile(sorted_returns, percentile_pos);
    } else if (method == "parametric") {
        double mu = mean(returns);
        double sigma = standard_deviation(returns);

        // Use normal distribution approximation
        // For 95% confidence: z-score ≈ -1.645
        double z_score = confidence_level == 0.95   ? -1.645
                         : confidence_level == 0.99 ? -2.326
                                                    : -1.645;

        return mu + z_score * sigma;
    } else if (method == "monte_carlo") {
        // Simplified Monte Carlo (using parametric for now)
        double mu = mean(returns);
        double sigma = standard_deviation(returns);
        double z_score = confidence_level == 0.95 ? -1.645 : -2.326;
        return mu + z_score * sigma;
    } else {
        throw std::invalid_argument("Unknown VaR method: " + method);
    }
}

double RiskManager::calculate_expected_shortfall(const std::vector<double>& returns,
                                                 double confidence_level) {
    if (returns.empty()) {
        throw std::invalid_argument("Returns vector cannot be empty");
    }

    std::vector<double> sorted_returns = returns;
    std::sort(sorted_returns.begin(), sorted_returns.end());

    size_t cutoff_index = static_cast<size_t>((1.0 - confidence_level) * returns.size());

    if (cutoff_index == 0) return sorted_returns[0];

    double sum = 0.0;
    for (size_t i = 0; i < cutoff_index; ++i) {
        sum += sorted_returns[i];
    }

    return sum / cutoff_index;
}

double RiskManager::calculate_sharpe_ratio(const std::vector<double>& returns,
                                           double risk_free_rate) {
    if (returns.empty()) return 0.0;

    double excess_return = mean(returns) - risk_free_rate / 252.0;  // Daily risk-free rate
    double volatility = standard_deviation(returns);

    return volatility != 0.0 ? (excess_return * std::sqrt(252.0)) / (volatility * std::sqrt(252.0))
                             : 0.0;
}

double RiskManager::calculate_max_drawdown(const std::vector<double>& prices) {
    if (prices.size() < 2) return 0.0;

    double max_drawdown = 0.0;
    double peak = prices[0];

    for (size_t i = 1; i < prices.size(); ++i) {
        if (prices[i] > peak) {
            peak = prices[i];
        }

        double drawdown = (peak - prices[i]) / peak;
        max_drawdown = std::max(max_drawdown, drawdown);
    }

    return max_drawdown * 100.0;  // Return as percentage
}

double RiskManager::calculate_calmar_ratio(const std::vector<double>& returns) {
    if (returns.empty()) return 0.0;

    // Calculate annualized return
    double total_return = 1.0;
    for (double ret : returns) {
        total_return *= (1.0 + ret);
    }
    double annualized_return = std::pow(total_return, 252.0 / returns.size()) - 1.0;

    // Convert returns to prices for max drawdown calculation
    std::vector<double> prices;
    prices.push_back(100.0);  // Starting price
    for (double ret : returns) {
        prices.push_back(prices.back() * (1.0 + ret));
    }

    double max_dd = calculate_max_drawdown(prices) / 100.0;  // Convert back to decimal

    return max_dd != 0.0 ? annualized_return / max_dd : 0.0;
}

double RiskManager::calculate_volatility(const std::vector<double>& returns, int trading_days) {
    if (returns.empty()) return 0.0;

    double daily_vol = standard_deviation(returns);
    return daily_vol * std::sqrt(trading_days);  // Annualize
}

double RiskManager::calculate_beta(const std::vector<double>& asset_returns,
                                   const std::vector<double>& market_returns) {
    if (asset_returns.size() != market_returns.size() || asset_returns.empty()) {
        return 1.0;  // Default beta
    }

    double cov = covariance(asset_returns, market_returns);
    double market_var = standard_deviation(market_returns);
    market_var *= market_var;  // Variance = std_dev^2

    return market_var != 0.0 ? cov / market_var : 1.0;
}

double RiskManager::calculate_tracking_error(const std::vector<double>& portfolio_returns,
                                             const std::vector<double>& benchmark_returns) {
    if (portfolio_returns.size() != benchmark_returns.size() || portfolio_returns.empty()) {
        return 0.0;
    }

    std::vector<double> excess_returns;
    for (size_t i = 0; i < portfolio_returns.size(); ++i) {
        excess_returns.push_back(portfolio_returns[i] - benchmark_returns[i]);
    }

    return standard_deviation(excess_returns) * std::sqrt(252.0);  // Annualized
}

double RiskManager::calculate_information_ratio(const std::vector<double>& portfolio_returns,
                                                const std::vector<double>& benchmark_returns) {
    if (portfolio_returns.size() != benchmark_returns.size() || portfolio_returns.empty()) {
        return 0.0;
    }

    std::vector<double> excess_returns;
    for (size_t i = 0; i < portfolio_returns.size(); ++i) {
        excess_returns.push_back(portfolio_returns[i] - benchmark_returns[i]);
    }

    double excess_return = mean(excess_returns) * 252.0;  // Annualized
    double tracking_error = standard_deviation(excess_returns) * std::sqrt(252.0);

    return tracking_error != 0.0 ? excess_return / tracking_error : 0.0;
}

// Helper functions
double RiskManager::percentile(std::vector<double> data, double p) {
    if (data.empty()) return 0.0;

    std::sort(data.begin(), data.end());

    if (p <= 0) return data[0];
    if (p >= 100) return data.back();

    double index = p / 100.0 * (data.size() - 1);
    size_t lower = static_cast<size_t>(std::floor(index));
    size_t upper = static_cast<size_t>(std::ceil(index));

    if (lower == upper) {
        return data[lower];
    }

    double weight = index - lower;
    return data[lower] * (1.0 - weight) + data[upper] * weight;
}

double RiskManager::mean(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

double RiskManager::standard_deviation(const std::vector<double>& data) {
    if (data.size() < 2) return 0.0;

    double mu = mean(data);
    double sum_sq_diff = 0.0;

    for (double value : data) {
        double diff = value - mu;
        sum_sq_diff += diff * diff;
    }

    return std::sqrt(sum_sq_diff / (data.size() - 1));  // Sample standard deviation
}

double RiskManager::covariance(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2) return 0.0;

    double mean_x = mean(x);
    double mean_y = mean(y);

    double sum = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        sum += (x[i] - mean_x) * (y[i] - mean_y);
    }

    return sum / (x.size() - 1);
}

// Portfolio Optimizer Implementation
std::vector<double> PortfolioOptimizer::optimize_mean_variance(
    const std::vector<std::vector<double>>& returns, const OptimizationConfig& config) {
    size_t n_assets = returns.size();
    if (n_assets == 0) return {};

    // Simple equal-weight portfolio for now
    std::vector<double> weights(n_assets, 1.0 / n_assets);

    std::cout << "Portfolio optimization: Equal weights assigned to " << n_assets << " assets"
              << std::endl;

    return weights;
}

std::vector<double> PortfolioOptimizer::optimize_risk_parity(
    const std::vector<std::vector<double>>& returns) {
    size_t n_assets = returns.size();
    if (n_assets == 0) return {};

    // Calculate volatilities
    std::vector<double> volatilities;
    for (const auto& asset_returns : returns) {
        volatilities.push_back(RiskManager::standard_deviation(asset_returns));
    }

    // Risk parity: weights inversely proportional to volatility
    std::vector<double> inv_vol_weights;
    double sum_inv_vol = 0.0;

    for (double vol : volatilities) {
        double inv_vol = vol > 0 ? 1.0 / vol : 1.0;
        inv_vol_weights.push_back(inv_vol);
        sum_inv_vol += inv_vol;
    }

    // Normalize weights
    for (double& weight : inv_vol_weights) {
        weight /= sum_inv_vol;
    }

    return inv_vol_weights;
}

std::vector<double> PortfolioOptimizer::optimize_minimum_variance(
    const std::vector<std::vector<double>>& returns) {
    size_t n_assets = returns.size();
    if (n_assets == 0) return {};

    // Simplified minimum variance (equal weights for now)
    // In practice, this would involve matrix operations
    std::vector<double> weights(n_assets, 1.0 / n_assets);

    std::cout << "Minimum variance optimization: Using equal weights (simplified)" << std::endl;

    return weights;
}

PortfolioMetrics PortfolioOptimizer::calculate_portfolio_metrics(
    const std::vector<std::vector<double>>& returns, const std::vector<double>& weights) {
    PortfolioMetrics metrics;

    if (returns.empty() || weights.empty() || returns.size() != weights.size()) {
        return metrics;  // Return default values
    }

    // Calculate portfolio returns
    std::vector<double> portfolio_returns;
    size_t n_periods = returns[0].size();

    for (size_t t = 0; t < n_periods; ++t) {
        double portfolio_return = 0.0;
        for (size_t i = 0; i < returns.size(); ++i) {
            if (t < returns[i].size()) {
                portfolio_return += weights[i] * returns[i][t];
            }
        }
        portfolio_returns.push_back(portfolio_return);
    }

    // Calculate metrics
    metrics.expected_return = RiskManager::mean(portfolio_returns) * 252.0;  // Annualized
    metrics.volatility = RiskManager::calculate_volatility(portfolio_returns);
    metrics.sharpe_ratio = RiskManager::calculate_sharpe_ratio(portfolio_returns);

    // Convert to prices for max drawdown
    std::vector<double> prices;
    prices.push_back(100.0);
    for (double ret : portfolio_returns) {
        prices.push_back(prices.back() * (1.0 + ret));
    }
    metrics.max_drawdown = RiskManager::calculate_max_drawdown(prices);

    metrics.var_95 = RiskManager::calculate_var(portfolio_returns, 0.95);

    return metrics;
}

}  // namespace stock_predict
