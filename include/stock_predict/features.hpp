#pragma once

#include "stock_predictor.hpp"
#include <vector>
#include <string>
#include <functional>

namespace stock_predict {

/**
 * @brief Technical analysis indicators
 */
class TechnicalIndicators {
public:
    /**
     * @brief Calculate Simple Moving Average
     * @param prices Price data
     * @param period Period for SMA
     * @return SMA values
     */
    static std::vector<double> sma(const std::vector<double>& prices, int period);
    
    /**
     * @brief Calculate Exponential Moving Average
     * @param prices Price data
     * @param period Period for EMA
     * @return EMA values
     */
    static std::vector<double> ema(const std::vector<double>& prices, int period);
    
    /**
     * @brief Calculate Relative Strength Index
     * @param prices Price data
     * @param period Period for RSI (default 14)
     * @return RSI values (0-100)
     */
    static std::vector<double> rsi(const std::vector<double>& prices, int period = 14);
    
    /**
     * @brief Calculate MACD (Moving Average Convergence Divergence)
     * @param prices Price data
     * @param fast_period Fast EMA period (default 12)
     * @param slow_period Slow EMA period (default 26)
     * @param signal_period Signal line period (default 9)
     * @return MACD line, signal line, and histogram
     */
    static std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> 
    macd(const std::vector<double>& prices, int fast_period = 12, int slow_period = 26, int signal_period = 9);
    
    /**
     * @brief Calculate Bollinger Bands
     * @param prices Price data
     * @param period Period for moving average (default 20)
     * @param std_dev Number of standard deviations (default 2.0)
     * @return Upper band, middle band (SMA), lower band
     */
    static std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
    bollinger_bands(const std::vector<double>& prices, int period = 20, double std_dev = 2.0);
    
    /**
     * @brief Calculate Stochastic Oscillator
     * @param high High prices
     * @param low Low prices
     * @param close Close prices
     * @param k_period %K period (default 14)
     * @param d_period %D period (default 3)
     * @return %K and %D values
     */
    static std::pair<std::vector<double>, std::vector<double>>
    stochastic(const std::vector<double>& high, const std::vector<double>& low,
               const std::vector<double>& close, int k_period = 14, int d_period = 3);
    
    /**
     * @brief Calculate Average True Range
     * @param high High prices
     * @param low Low prices  
     * @param close Close prices
     * @param period Period for ATR (default 14)
     * @return ATR values
     */
    static std::vector<double> atr(const std::vector<double>& high, const std::vector<double>& low,
                                   const std::vector<double>& close, int period = 14);
    
    /**
     * @brief Calculate Williams %R
     * @param high High prices
     * @param low Low prices
     * @param close Close prices
     * @param period Period for Williams %R (default 14)
     * @return Williams %R values
     */
    static std::vector<double> williams_r(const std::vector<double>& high, const std::vector<double>& low,
                                          const std::vector<double>& close, int period = 14);
    
    /**
     * @brief Calculate Commodity Channel Index
     * @param high High prices
     * @param low Low prices
     * @param close Close prices
     * @param period Period for CCI (default 20)
     * @return CCI values
     */
    static std::vector<double> cci(const std::vector<double>& high, const std::vector<double>& low,
                                   const std::vector<double>& close, int period = 20);
};

/**
 * @brief Feature engineering for market data
 */
class FeatureEngineer {
public:
    /**
     * @brief Extract comprehensive features from market data
     * @param data Market data
     * @return Feature matrix (each row is a time point, columns are features)
     */
    static std::vector<std::vector<double>> extract_features(const std::vector<MarketData>& data);
    
    /**
     * @brief Extract price-based features
     * @param data Market data
     * @return Price features
     */
    static std::vector<std::vector<double>> extract_price_features(const std::vector<MarketData>& data);
    
    /**
     * @brief Extract volume-based features
     * @param data Market data
     * @return Volume features
     */
    static std::vector<std::vector<double>> extract_volume_features(const std::vector<MarketData>& data);
    
    /**
     * @brief Extract technical indicator features
     * @param data Market data
     * @return Technical indicator features
     */
    static std::vector<std::vector<double>> extract_technical_features(const std::vector<MarketData>& data);
    
    /**
     * @brief Extract volatility features
     * @param data Market data
     * @return Volatility features
     */
    static std::vector<std::vector<double>> extract_volatility_features(const std::vector<MarketData>& data);
    
    /**
     * @brief Extract time-based features (day of week, month, etc.)
     * @param data Market data
     * @return Time-based features
     */
    static std::vector<std::vector<double>> extract_time_features(const std::vector<MarketData>& data);
    
    /**
     * @brief Apply log transformation to prices
     * @param prices Input prices
     * @return Log-transformed prices
     */
    static std::vector<double> log_transform(const std::vector<double>& prices);
    
    /**
     * @brief Calculate price returns
     * @param prices Input prices
     * @param periods Number of periods for return calculation (default 1)
     * @return Price returns
     */
    static std::vector<double> calculate_returns(const std::vector<double>& prices, int periods = 1);
    
    /**
     * @brief Normalize features using z-score
     * @param features Input features
     * @return Normalized features and normalization parameters (mean, std)
     */
    static std::pair<std::vector<std::vector<double>>, std::pair<std::vector<double>, std::vector<double>>>
    normalize_features(const std::vector<std::vector<double>>& features);
    
    /**
     * @brief Create lag features
     * @param data Input data
     * @param lags Vector of lag values
     * @return Lagged features
     */
    static std::vector<std::vector<double>> create_lag_features(
        const std::vector<double>& data, const std::vector<int>& lags);
    
    /**
     * @brief Create rolling window statistics
     * @param data Input data
     * @param window_size Window size for rolling statistics
     * @return Rolling mean, std, min, max
     */
    static std::tuple<std::vector<double>, std::vector<double>, std::vector<double>, std::vector<double>>
    rolling_statistics(const std::vector<double>& data, int window_size);
};

/**
 * @brief Wavelet transform for time series analysis
 */
class WaveletTransform {
public:
    /**
     * @brief Apply Discrete Wavelet Transform
     * @param signal Input signal
     * @param wavelet Wavelet type ("haar", "db4", "db8")
     * @param levels Number of decomposition levels
     * @return Wavelet coefficients
     */
    static std::vector<std::vector<double>> dwt(const std::vector<double>& signal,
                                                const std::string& wavelet = "db4",
                                                int levels = 3);
    
    /**
     * @brief Apply Inverse Discrete Wavelet Transform
     * @param coefficients Wavelet coefficients
     * @param wavelet Wavelet type
     * @return Reconstructed signal
     */
    static std::vector<double> idwt(const std::vector<std::vector<double>>& coefficients,
                                    const std::string& wavelet = "db4");
    
    /**
     * @brief Denoise signal using wavelet denoising
     * @param signal Input signal
     * @param threshold Threshold for coefficient filtering
     * @param wavelet Wavelet type
     * @return Denoised signal
     */
    static std::vector<double> denoise(const std::vector<double>& signal,
                                       double threshold = 0.1,
                                       const std::string& wavelet = "db4");
};

/**
 * @brief Fourier transform for frequency domain analysis
 */
class FourierTransform {
public:
    /**
     * @brief Apply Fast Fourier Transform
     * @param signal Input signal (real values)
     * @return Complex frequency domain representation
     */
    static std::vector<std::complex<double>> fft(const std::vector<double>& signal);
    
    /**
     * @brief Apply Inverse Fast Fourier Transform
     * @param spectrum Frequency domain representation
     * @return Time domain signal
     */
    static std::vector<double> ifft(const std::vector<std::complex<double>>& spectrum);
    
    /**
     * @brief Calculate power spectral density
     * @param signal Input signal
     * @return Power spectral density
     */
    static std::vector<double> power_spectrum(const std::vector<double>& signal);
    
    /**
     * @brief Extract dominant frequencies
     * @param signal Input signal
     * @param sample_rate Sampling rate
     * @param num_frequencies Number of top frequencies to extract
     * @return Dominant frequencies and their magnitudes
     */
    static std::vector<std::pair<double, double>> dominant_frequencies(
        const std::vector<double>& signal, double sample_rate, int num_frequencies = 5);
};

} // namespace stock_predict
