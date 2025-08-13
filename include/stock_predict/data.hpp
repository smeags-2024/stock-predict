#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "stock_predictor.hpp"

namespace stock_predict {

/**
 * @brief Data source interface for market data
 */
class IDataSource {
   public:
    virtual ~IDataSource() = default;

    /**
     * @brief Get historical market data
     * @param symbol Stock symbol
     * @param start_date Start date
     * @param end_date End date
     * @return Historical market data
     */
    virtual std::vector<MarketData> get_historical_data(
        const std::string& symbol, const std::chrono::system_clock::time_point& start_date,
        const std::chrono::system_clock::time_point& end_date) = 0;

    /**
     * @brief Get real-time market data
     * @param symbol Stock symbol
     * @return Latest market data
     */
    virtual std::optional<MarketData> get_real_time_data(const std::string& symbol) = 0;

    /**
     * @brief Check if data source is available
     * @return true if available, false otherwise
     */
    virtual bool is_available() const = 0;
};

/**
 * @brief CSV file data source
 */
class CSVDataSource : public IDataSource {
   public:
    explicit CSVDataSource(const std::string& file_path);

    std::vector<MarketData> get_historical_data(
        const std::string& symbol, const std::chrono::system_clock::time_point& start_date,
        const std::chrono::system_clock::time_point& end_date) override;

    std::optional<MarketData> get_real_time_data(const std::string& symbol) override;
    bool is_available() const override;

   private:
    std::string file_path_;
    std::vector<MarketData> cached_data_;
    bool load_data();
};

/**
 * @brief Alpha Vantage API data source
 */
class AlphaVantageDataSource : public IDataSource {
   public:
    explicit AlphaVantageDataSource(const std::string& api_key);

    std::vector<MarketData> get_historical_data(
        const std::string& symbol, const std::chrono::system_clock::time_point& start_date,
        const std::chrono::system_clock::time_point& end_date) override;

    std::optional<MarketData> get_real_time_data(const std::string& symbol) override;
    bool is_available() const override;

   private:
    std::string api_key_;
    std::string make_request(const std::string& url);
};

/**
 * @brief Yahoo Finance data source
 */
class YahooFinanceDataSource : public IDataSource {
   public:
    YahooFinanceDataSource() = default;

    std::vector<MarketData> get_historical_data(
        const std::string& symbol, const std::chrono::system_clock::time_point& start_date,
        const std::chrono::system_clock::time_point& end_date) override;

    std::optional<MarketData> get_real_time_data(const std::string& symbol) override;
    bool is_available() const override;

   private:
    std::string make_request(const std::string& url);
};

/**
 * @brief Data manager for coordinating multiple data sources
 */
class DataManager {
   public:
    DataManager();

    /**
     * @brief Add a data source
     * @param source Data source to add
     * @param priority Priority level (higher = preferred)
     */
    void add_data_source(std::unique_ptr<IDataSource> source, int priority = 0);

    /**
     * @brief Get historical data from the best available source
     * @param symbol Stock symbol
     * @param start_date Start date
     * @param end_date End date
     * @return Historical market data
     */
    std::vector<MarketData> get_historical_data(
        const std::string& symbol, const std::chrono::system_clock::time_point& start_date,
        const std::chrono::system_clock::time_point& end_date);

    /**
     * @brief Get real-time data from the best available source
     * @param symbol Stock symbol
     * @return Latest market data
     */
    std::optional<MarketData> get_real_time_data(const std::string& symbol);

    /**
     * @brief Cache data to local storage
     * @param symbol Stock symbol
     * @param data Market data to cache
     * @param cache_path Path to cache directory
     */
    void cache_data(const std::string& symbol, const std::vector<MarketData>& data,
                    const std::string& cache_path = "cache/");

    /**
     * @brief Load data from cache
     * @param symbol Stock symbol
     * @param cache_path Path to cache directory
     * @return Cached market data
     */
    std::vector<MarketData> load_cached_data(const std::string& symbol,
                                             const std::string& cache_path = "cache/");

   private:
    std::vector<std::pair<std::unique_ptr<IDataSource>, int>> data_sources_;

    void sort_sources_by_priority();
};

/**
 * @brief Data preprocessing utilities
 */
class DataProcessor {
   public:
    /**
     * @brief Clean market data (remove outliers, fill missing values)
     * @param data Input market data
     * @param method Cleaning method ("drop", "interpolate", "forward_fill")
     * @return Cleaned market data
     */
    static std::vector<MarketData> clean_data(const std::vector<MarketData>& data,
                                              const std::string& method = "interpolate");

    /**
     * @brief Resample data to different frequency
     * @param data Input market data
     * @param frequency Target frequency ("1D", "1H", "1min")
     * @return Resampled market data
     */
    static std::vector<MarketData> resample_data(const std::vector<MarketData>& data,
                                                 const std::string& frequency = "1D");

    /**
     * @brief Split data into train/validation/test sets
     * @param data Input market data
     * @param train_ratio Training data ratio
     * @param val_ratio Validation data ratio
     * @return Train, validation, and test datasets
     */
    static std::tuple<std::vector<MarketData>, std::vector<MarketData>, std::vector<MarketData>>
    split_data(const std::vector<MarketData>& data, double train_ratio = 0.7,
               double val_ratio = 0.15);

    /**
     * @brief Detect and remove outliers
     * @param data Input market data
     * @param method Outlier detection method ("iqr", "zscore", "isolation_forest")
     * @param threshold Threshold for outlier detection
     * @return Data with outliers removed
     */
    static std::vector<MarketData> remove_outliers(const std::vector<MarketData>& data,
                                                   const std::string& method = "iqr",
                                                   double threshold = 3.0);

    /**
     * @brief Align multiple time series by timestamp
     * @param datasets Vector of market data for different symbols
     * @return Aligned datasets
     */
    static std::vector<std::vector<MarketData>> align_timestamps(
        const std::vector<std::vector<MarketData>>& datasets);

    /**
     * @brief Calculate data quality metrics
     * @param data Input market data
     * @return Quality metrics (completeness, consistency, etc.)
     */
    static std::vector<std::pair<std::string, double>> calculate_quality_metrics(
        const std::vector<MarketData>& data);

   private:
    static std::vector<MarketData> interpolate_missing(const std::vector<MarketData>& data);
    static std::vector<MarketData> forward_fill(const std::vector<MarketData>& data);
    static bool is_outlier_iqr(double value, const std::vector<double>& values);
    static bool is_outlier_zscore(double value, const std::vector<double>& values,
                                  double threshold);
};

}  // namespace stock_predict
