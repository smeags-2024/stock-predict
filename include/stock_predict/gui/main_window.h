#pragma once

#include <QtCore/QMutex>
#include <QtCore/QThread>

// Only include Qt Charts if available
#if defined(QT_CHARTS_LIB) && __has_include(<QtCharts/QChart>)
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#define HAS_QT_CHARTS
#endif
#include <QtCore/QTimer>
#include <QtWidgets/QAction>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <memory>

#include "stock_predict/data.hpp"
#include "stock_predict/risk.hpp"
#include "stock_predict/stock_predictor.hpp"

namespace stock_predict {
namespace gui {

/**
 * @brief Worker thread for running predictions without blocking the UI
 */
class PredictionWorker : public QObject {
    Q_OBJECT

   public:
    PredictionWorker(std::unique_ptr<IStockPredictor> predictor, std::vector<MarketData> data,
                     int prediction_days = 1);

   public slots:
    void runPrediction();
    void runTraining(int epochs);

   signals:
    void predictionComplete(const std::vector<PredictionResult>& results);
    void trainingProgress(int epoch, double loss);
    void trainingComplete();
    void errorOccurred(const QString& error);

   private:
    std::unique_ptr<IStockPredictor> predictor_;
    std::vector<MarketData> data_;
    int prediction_days_;
    QMutex mutex_;
};

/**
 * @brief Real-time data updater thread
 */
class DataUpdateWorker : public QObject {
    Q_OBJECT

   public:
    DataUpdateWorker(std::unique_ptr<DataManager> data_manager,
                     const std::vector<std::string>& symbols);

   public slots:
    void startUpdates();
    void stopUpdates();

   signals:
    void dataUpdated(const std::string& symbol, const MarketData& data);
    void errorOccurred(const QString& error);

   private:
    std::unique_ptr<DataManager> data_manager_;
    std::vector<std::string> symbols_;
    QTimer* update_timer_;
    bool running_;
};

/**
 * @brief Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

   protected:
    void closeEvent(QCloseEvent* event) override;

   private slots:
    // Prediction actions
    void onLoadData();
    void onPredict();
    void onTrain();
    void onBacktest();

    // Model management
    void onModelTypeChanged();
    void onLoadModel();
    void onSaveModel();

    // Data management
    void onSymbolChanged();
    void onRefreshData();
    void onExportData();

    // Real-time updates
    void onStartRealTime();
    void onStopRealTime();
    void onDataUpdated(const std::string& symbol, const MarketData& data);

    // Results handling
    void onPredictionComplete(const std::vector<PredictionResult>& results);
    void onTrainingProgress(int epoch, double loss);
    void onTrainingComplete();
    void onErrorOccurred(const QString& error);

    // UI updates
    void updateCharts();
    void updateMetrics();
    void updateStatus(const QString& message);

   private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();

    // UI creation helpers
    QWidget* createControlPanel();
    QWidget* createChartsPanel();
    QWidget* createMetricsPanel();
    QWidget* createLogPanel();
    QTabWidget* createMainTabs();

    // Chart helpers
    void setupPriceChart();
    void setupPredictionChart();
    void setupRiskChart();
    void addPriceData(const std::vector<MarketData>& data);
    void addPredictionData(const std::vector<PredictionResult>& predictions);

    // Data helpers
    void loadHistoricalData();
    void validateInputs();
    std::vector<MarketData> getCurrentData() const;

    // Threading
    void setupWorkerThreads();
    void cleanupWorkerThreads();

   private:
    // Core components
    std::unique_ptr<IStockPredictor> predictor_;
    std::unique_ptr<DataManager> data_manager_;
    std::vector<MarketData> current_data_;
    std::vector<PredictionResult> current_predictions_;

    // UI Components - Control Panel
    QComboBox* symbol_combo_;
    QComboBox* model_combo_;
    QSpinBox* sequence_length_spin_;
    QSpinBox* prediction_days_spin_;
    QSpinBox* epochs_spin_;
    QCheckBox* gpu_checkbox_;
    QCheckBox* realtime_checkbox_;
    QPushButton* load_data_button_;
    QPushButton* predict_button_;
    QPushButton* train_button_;
    QPushButton* backtest_button_;
    QPushButton* load_model_button_;
    QPushButton* save_model_button_;
    QProgressBar* progress_bar_;

    // UI Components - Charts
    QTabWidget* main_tabs_;
    QtCharts::QChart* price_chart_;
    QtCharts::QChart* prediction_chart_;
    QtCharts::QChart* risk_chart_;
    QtCharts::QChartView* price_chart_view_;
    QtCharts::QChartView* prediction_chart_view_;
    QtCharts::QChartView* risk_chart_view_;

    // UI Components - Data Display
    QTableWidget* metrics_table_;
    QTableWidget* predictions_table_;
    QTableWidget* risk_table_;
    QTextEdit* log_text_;

    // UI Components - Layout
    QSplitter* main_splitter_;
    QSplitter* charts_splitter_;

    // Threading
    QThread* prediction_thread_;
    QThread* data_update_thread_;
    PredictionWorker* prediction_worker_;
    DataUpdateWorker* data_update_worker_;

    // Menu and toolbar
    QMenuBar* menu_bar_;
    QToolBar* tool_bar_;
    QStatusBar* status_bar_;
    QLabel* status_label_;
    QLabel* connection_status_;

    // Actions
    QAction* new_action_;
    QAction* open_action_;
    QAction* save_action_;
    QAction* exit_action_;
    QAction* about_action_;
    QAction* settings_action_;

    // State
    bool is_training_;
    bool is_predicting_;
    bool realtime_active_;
    QString current_symbol_;
    QString current_model_path_;
};

/**
 * @brief Settings dialog for configuring application parameters
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

   public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // Getters for settings
    PredictorConfig getPredictorConfig() const;
    QString getDataSourceConfig() const;
    QString getApiKey() const;
    bool getAutoSave() const;
    int getUpdateInterval() const;

   public slots:
    void accept() override;
    void reject() override;

   private:
    void setupUI();
    void loadSettings();
    void saveSettings();

   private:
    // Model settings
    QComboBox* model_type_combo_;
    QSpinBox* sequence_length_spin_;
    QSpinBox* hidden_size_spin_;
    QSpinBox* num_layers_spin_;
    QDoubleSpinBox* dropout_spin_;
    QDoubleSpinBox* learning_rate_spin_;

    // Data settings
    QComboBox* data_source_combo_;
    QLineEdit* api_key_edit_;
    QLineEdit* cache_path_edit_;
    QSpinBox* cache_ttl_spin_;

    // Performance settings
    QCheckBox* gpu_checkbox_;
    QSpinBox* threads_spin_;
    QSpinBox* batch_size_spin_;

    // UI settings
    QCheckBox* auto_save_checkbox_;
    QSpinBox* update_interval_spin_;
    QCheckBox* show_tooltips_checkbox_;
    QComboBox* theme_combo_;
};

/**
 * @brief About dialog
 */
class AboutDialog : public QDialog {
    Q_OBJECT

   public:
    explicit AboutDialog(QWidget* parent = nullptr);

   private:
    void setupUI();
};

/**
 * @brief Portfolio analysis widget
 */
class PortfolioWidget : public QWidget {
    Q_OBJECT

   public:
    explicit PortfolioWidget(QWidget* parent = nullptr);

    void setPortfolioData(const std::vector<std::string>& symbols,
                          const std::vector<double>& weights,
                          const std::vector<std::vector<double>>& returns);

   private slots:
    void onOptimizePortfolio();
    void onRebalance();
    void onAddAsset();
    void onRemoveAsset();

   private:
    void setupUI();
    void updatePortfolioMetrics();

   private:
    QTableWidget* holdings_table_;
    QTableWidget* metrics_table_;
    QtCharts::QChart* allocation_chart_;
    QtCharts::QChart* performance_chart_;
    QPushButton* optimize_button_;
    QPushButton* rebalance_button_;

    std::vector<std::string> symbols_;
    std::vector<double> weights_;
    std::vector<std::vector<double>> returns_;
};

/**
 * @brief Risk analysis widget
 */
class RiskWidget : public QWidget {
    Q_OBJECT

   public:
    explicit RiskWidget(QWidget* parent = nullptr);

    void updateRiskMetrics(const std::vector<double>& returns, const std::vector<double>& prices);

   private:
    void setupUI();
    void calculateRiskMetrics();

   private:
    QTableWidget* risk_metrics_table_;
    QtCharts::QChart* var_chart_;
    QtCharts::QChart* drawdown_chart_;
    QProgressBar* risk_score_bar_;

    std::vector<double> returns_;
    std::vector<double> prices_;
};

}  // namespace gui
}  // namespace stock_predict
