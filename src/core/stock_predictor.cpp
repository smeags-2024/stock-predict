#include "stock_predict/stock_predictor.hpp"
#include "stock_predict/models.hpp"
#include <stdexcept>

namespace stock_predict {

std::unique_ptr<IStockPredictor> create_predictor(
    const std::string& symbol, 
    const PredictorConfig& config) {
    
    if (config.model_type == "lstm") {
        return std::make_unique<LSTMPredictor>(symbol, config);
    } else if (config.model_type == "gru") {
        // For now, use LSTM as GRU placeholder
        return std::make_unique<LSTMPredictor>(symbol, config);
    } else if (config.model_type == "transformer") {
        return std::make_unique<TransformerPredictor>(symbol, config);
    } else if (config.model_type == "ensemble") {
        return std::make_unique<EnsemblePredictor>(symbol, config);
    } else {
        throw std::invalid_argument("Unknown model type: " + config.model_type);
    }
}

} // namespace stock_predict
