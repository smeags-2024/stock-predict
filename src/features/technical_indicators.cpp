#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include "stock_predict/features.hpp"

namespace stock_predict {

// Technical Indicators Implementation

std::vector<double> TechnicalIndicators::sma(const std::vector<double>& prices, int period) {
    if (prices.size() < static_cast<size_t>(period)) {
        return {};
    }

    std::vector<double> result;
    result.reserve(prices.size() - period + 1);

    for (size_t i = period - 1; i < prices.size(); ++i) {
        double sum = 0.0;
        for (int j = 0; j < period; ++j) {
            sum += prices[i - j];
        }
        result.push_back(sum / period);
    }

    return result;
}

std::vector<double> TechnicalIndicators::ema(const std::vector<double>& prices, int period) {
    if (prices.empty()) return {};

    std::vector<double> result;
    result.reserve(prices.size());

    double multiplier = 2.0 / (period + 1);

    // First EMA is just the first price
    result.push_back(prices[0]);

    for (size_t i = 1; i < prices.size(); ++i) {
        double ema_value = (prices[i] * multiplier) + (result[i - 1] * (1 - multiplier));
        result.push_back(ema_value);
    }

    return result;
}

std::vector<double> TechnicalIndicators::rsi(const std::vector<double>& prices, int period) {
    if (prices.size() < static_cast<size_t>(period + 1)) {
        return {};
    }

    std::vector<double> gains, losses;

    // Calculate price changes
    for (size_t i = 1; i < prices.size(); ++i) {
        double change = prices[i] - prices[i - 1];
        gains.push_back(change > 0 ? change : 0);
        losses.push_back(change < 0 ? -change : 0);
    }

    std::vector<double> result;
    result.reserve(gains.size() - period + 1);

    // Calculate initial average gain and loss
    double avg_gain = 0.0, avg_loss = 0.0;
    for (int i = 0; i < period; ++i) {
        avg_gain += gains[i];
        avg_loss += losses[i];
    }
    avg_gain /= period;
    avg_loss /= period;

    // Calculate RSI
    for (size_t i = period; i < gains.size(); ++i) {
        if (i > static_cast<size_t>(period)) {
            // Smoothed averages
            avg_gain = ((avg_gain * (period - 1)) + gains[i]) / period;
            avg_loss = ((avg_loss * (period - 1)) + losses[i]) / period;
        }

        double rs = avg_loss != 0 ? avg_gain / avg_loss : 100.0;
        double rsi_value = 100.0 - (100.0 / (1.0 + rs));
        result.push_back(rsi_value);
    }

    return result;
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> TechnicalIndicators::macd(
    const std::vector<double>& prices, int fast_period, int slow_period, int signal_period) {
    auto fast_ema = ema(prices, fast_period);
    auto slow_ema = ema(prices, slow_period);

    std::vector<double> macd_line, signal_line, histogram;

    if (fast_ema.size() < slow_ema.size()) {
        return {macd_line, signal_line, histogram};
    }

    // Calculate MACD line
    size_t start_idx = fast_ema.size() - slow_ema.size();
    for (size_t i = 0; i < slow_ema.size(); ++i) {
        macd_line.push_back(fast_ema[start_idx + i] - slow_ema[i]);
    }

    // Calculate signal line (EMA of MACD line)
    signal_line = ema(macd_line, signal_period);

    // Calculate histogram (MACD - Signal)
    size_t hist_start = macd_line.size() - signal_line.size();
    for (size_t i = 0; i < signal_line.size(); ++i) {
        histogram.push_back(macd_line[hist_start + i] - signal_line[i]);
    }

    return {macd_line, signal_line, histogram};
}

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
TechnicalIndicators::bollinger_bands(const std::vector<double>& prices, int period,
                                     double std_dev) {
    auto middle_band = sma(prices, period);
    std::vector<double> upper_band, lower_band;

    if (middle_band.empty()) {
        return {upper_band, middle_band, lower_band};
    }

    upper_band.reserve(middle_band.size());
    lower_band.reserve(middle_band.size());

    for (size_t i = 0; i < middle_band.size(); ++i) {
        size_t start_idx = i + period - 1;

        // Calculate standard deviation for this window
        double sum_sq_diff = 0.0;
        for (int j = 0; j < period; ++j) {
            double diff = prices[start_idx - j] - middle_band[i];
            sum_sq_diff += diff * diff;
        }
        double std_deviation = std::sqrt(sum_sq_diff / period);

        upper_band.push_back(middle_band[i] + (std_dev * std_deviation));
        lower_band.push_back(middle_band[i] - (std_dev * std_deviation));
    }

    return {upper_band, middle_band, lower_band};
}

std::pair<std::vector<double>, std::vector<double>> TechnicalIndicators::stochastic(
    const std::vector<double>& high, const std::vector<double>& low,
    const std::vector<double>& close, int k_period, int d_period) {
    if (high.size() != low.size() || low.size() != close.size() ||
        close.size() < static_cast<size_t>(k_period)) {
        return {{}, {}};
    }

    std::vector<double> k_values, d_values;

    // Calculate %K
    for (size_t i = k_period - 1; i < close.size(); ++i) {
        double highest_high =
            *std::max_element(high.begin() + i - k_period + 1, high.begin() + i + 1);
        double lowest_low = *std::min_element(low.begin() + i - k_period + 1, low.begin() + i + 1);

        double k_value = 100.0 * (close[i] - lowest_low) / (highest_high - lowest_low);
        k_values.push_back(k_value);
    }

    // Calculate %D (SMA of %K)
    d_values = sma(k_values, d_period);

    return {k_values, d_values};
}

std::vector<double> TechnicalIndicators::atr(const std::vector<double>& high,
                                             const std::vector<double>& low,
                                             const std::vector<double>& close, int period) {
    if (high.size() != low.size() || low.size() != close.size() || close.size() < 2) {
        return {};
    }

    std::vector<double> true_ranges;

    for (size_t i = 1; i < close.size(); ++i) {
        double tr1 = high[i] - low[i];
        double tr2 = std::abs(high[i] - close[i - 1]);
        double tr3 = std::abs(low[i] - close[i - 1]);

        double true_range = std::max({tr1, tr2, tr3});
        true_ranges.push_back(true_range);
    }

    return sma(true_ranges, period);
}

std::vector<double> TechnicalIndicators::williams_r(const std::vector<double>& high,
                                                    const std::vector<double>& low,
                                                    const std::vector<double>& close, int period) {
    if (high.size() != low.size() || low.size() != close.size() ||
        close.size() < static_cast<size_t>(period)) {
        return {};
    }

    std::vector<double> result;

    for (size_t i = period - 1; i < close.size(); ++i) {
        double highest_high =
            *std::max_element(high.begin() + i - period + 1, high.begin() + i + 1);
        double lowest_low = *std::min_element(low.begin() + i - period + 1, low.begin() + i + 1);

        double williams_r_value = -100.0 * (highest_high - close[i]) / (highest_high - lowest_low);
        result.push_back(williams_r_value);
    }

    return result;
}

// Feature Engineer Implementation
std::vector<std::vector<double>> FeatureEngineer::extract_features(
    const std::vector<MarketData>& data) {
    std::vector<std::vector<double>> features;

    if (data.empty()) return features;

    // Extract price vectors
    std::vector<double> opens, highs, lows, closes, volumes;
    for (const auto& point : data) {
        opens.push_back(point.open);
        highs.push_back(point.high);
        lows.push_back(point.low);
        closes.push_back(point.close);
        volumes.push_back(point.volume);
    }

    // Calculate technical indicators
    std::vector<double> sma20 = TechnicalIndicators::sma(closes, 20);
    std::vector<double> ema12 = TechnicalIndicators::ema(closes, 12);
    std::vector<double> rsi14 = TechnicalIndicators::rsi(closes, 14);
    std::vector<double> volume_sma = TechnicalIndicators::sma(volumes, 20);

    // Create feature matrix - each row is one time point's features
    for (size_t i = 20; i < data.size(); ++i) {  // Start from index 20 to have enough data for indicators
        std::vector<double> feature_row;

        // Price features
        if (i > 0) {
            feature_row.push_back((closes[i] - closes[i - 1]) / closes[i - 1]);  // Returns
            feature_row.push_back((highs[i] - lows[i]) / closes[i]);             // Volatility
            feature_row.push_back(volumes[i] / volume_sma[i - 20]);              // Volume ratio
        }

        // Technical indicators (adjust indices for the vectors)
        if (i - 20 < sma20.size()) {
            feature_row.push_back(sma20[i - 20]);
        }
        if (i - 12 < ema12.size()) {
            feature_row.push_back(ema12[i - 12]);
        }
        if (i - 14 < rsi14.size()) {
            feature_row.push_back(rsi14[i - 14]);
        }

        features.push_back(feature_row);
    }

    return features;
}
        if (i < sma20.size()) feature_row.push_back(sma20[i]);
        if (i < ema12.size()) feature_row.push_back(ema12[i]);
        if (i < rsi.size()) feature_row.push_back(rsi[i]);

        features.push_back(feature_row);
    }

    return features;
}

std::pair<std::vector<std::vector<double>>, std::pair<std::vector<double>, std::vector<double>>>
FeatureEngineer::normalize_features(const std::vector<std::vector<double>>& features) {
    std::vector<std::vector<double>> normalized;
    std::vector<double> means, stds;
    
    if (features.empty()) {
        return {normalized, {means, stds}};
    }

    if (features[0].empty()) {
        return {features, {means, stds}};
    }

    size_t num_features = features[0].size();
    normalized.resize(features.size(), std::vector<double>(num_features));
    means.resize(num_features);
    stds.resize(num_features);

    // Normalize each feature column independently
    for (size_t feature_idx = 0; feature_idx < num_features; ++feature_idx) {
        // Calculate mean for this feature
        double mean = 0.0;
        for (size_t sample_idx = 0; sample_idx < features.size(); ++sample_idx) {
            mean += features[sample_idx][feature_idx];
        }
        mean /= features.size();
        means[feature_idx] = mean;

        // Calculate standard deviation
        double variance = 0.0;
        for (size_t sample_idx = 0; sample_idx < features.size(); ++sample_idx) {
            double diff = features[sample_idx][feature_idx] - mean;
            variance += diff * diff;
        }
        variance /= features.size();
        double std_dev = std::sqrt(variance);
        stds[feature_idx] = std_dev;

        // Normalize this feature across all samples
        for (size_t sample_idx = 0; sample_idx < features.size(); ++sample_idx) {
            normalized[sample_idx][feature_idx] =
                std_dev > 0 ? (features[sample_idx][feature_idx] - mean) / std_dev : 0.0;
        }
    }

    return {normalized, {means, stds}};
}

}  // namespace stock_predict
