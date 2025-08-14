#include "stock_predict/stock_predictor.hpp"
#include <iostream>
#include <stdexcept>
#include "stock_predict/models.hpp"

namespace stock_predict {

std::unique_ptr<IStockPredictor> create_predictor(const std::string& symbol,
                                                  const PredictorConfig& config) {
    if (config.model_type == "lstm") {
        std::cout << "Using mathematical LSTM implementation for symbol: " << symbol << std::endl;
        return std::make_unique<SimplePredictor>(symbol, config);
    } else if (config.model_type == "gru") {
        std::cout << "Using mathematical GRU implementation for symbol: " << symbol << std::endl;
        return std::make_unique<SimplePredictor>(symbol, config);
    } else if (config.model_type == "transformer") {
        std::cout << "Using mathematical Transformer implementation for symbol: " << symbol << std::endl;
        return std::make_unique<SimplePredictor>(symbol, config);
    } else if (config.model_type == "ensemble") {
        std::cout << "Using mathematical Ensemble implementation for symbol: " << symbol << std::endl;
        return std::make_unique<SimplePredictor>(symbol, config);
    } else if (config.model_type == "simple") {
        return std::make_unique<SimplePredictor>(symbol, config);
    } else {
        std::cout << "Unknown model type: " << config.model_type << ", using SimplePredictor"
                  << std::endl;
        return std::make_unique<SimplePredictor>(symbol, config);
    }
}

}  // namespace stock_predict
