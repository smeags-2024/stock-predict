#pragma once

#include <QAction>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDateTimeAxis>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLineSeries>
#include <QMainWindow>
#include <QMenuBar>
#include <QMutex>
#include <QProgressBar>
#include <QPushButton>
#include <QScatterSeries>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QWidget>
#include <memory>

#include "stock_predict/data.hpp"
#include "stock_predict/risk.hpp"
#include "stock_predict/stock_predictor.hpp"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class PredictionWorker;
class RealTimeUpdater;

/**
 * @brief Main window for the Stock Prediction GUI application
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

   private slots:
    // Menu actions
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void showPreferences();
    void showAbout();

    // Main functionality
    void loadData();
    void trainModel();
    void makePrediction();
    void startRealTimeMode();
    void stopRealTimeMode();
    void runBacktest();
    void optimizePortfolio();

    // Data handling
    void onDataLoaded(const std::vector<stock_predict::MarketData>& data);
    void onModelTrained(bool success, const QString& message);
    void onPredictionReady(const stock_predict::PredictionResult& result);
    void onRealTimeUpdate(const stock_predict::MarketData& data);
    void onBacktestComplete(const QVariantMap& results);

    // UI updates
    void updateStatusBar(const QString& message);
    void updateProgress(int value);
    void updateModelMetrics();
    void updateRiskMetrics();
    void refreshCharts();

    // Settings
    void applySettings();
    void resetSettings();

   private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupTabs();
    void setupPredictionTab();
    void setupTrainingTab();
    void setupBacktestTab();
    void setupPortfolioTab();
    void setupRiskTab();
    void setupChartsTab();
    void setupSettingsTab();

    void setupConnections();
    void loadSettings();
    void saveSettings();

    void updateChart(QChart* chart, const std::vector<stock_predict::MarketData>& data);
    void updatePredictionChart(const stock_predict::PredictionResult& prediction);
    void updateRiskChart(const std::vector<double>& risks);

    QString formatCurrency(double value) const;
    QString formatPercentage(double value) const;
    QString formatDateTime(const std::chrono::system_clock::time_point& time) const;

    // UI Components
    QTabWidget* m_centralTabs;

    // Prediction Tab
    QWidget* m_predictionTab;
    QLineEdit* m_symbolEdit;
    QComboBox* m_modelCombo;
    QSpinBox* m_predictionDaysSpinBox;
    QPushButton* m_predictButton;
    QPushButton* m_realTimeButton;
    QLabel* m_currentPriceLabel;
    QLabel* m_predictedPriceLabel;
    QLabel* m_confidenceLabel;
    QLabel* m_returnLabel;
    QTableWidget* m_predictionTable;
    QChartView* m_predictionChartView;
    QChart* m_predictionChart;

    // Training Tab
    QWidget* m_trainingTab;
    QLineEdit* m_dataSourceEdit;
    QPushButton* m_loadDataButton;
    QSpinBox* m_epochsSpinBox;
    QDoubleSpinBox* m_learningRateSpinBox;
    QPushButton* m_trainButton;
    QProgressBar* m_trainingProgress;
    QTextEdit* m_trainingLog;
    QTableWidget* m_metricsTable;

    // Backtest Tab
    QWidget* m_backtestTab;
    QLineEdit* m_startDateEdit;
    QLineEdit* m_endDateEdit;
    QDoubleSpinBox* m_initialCapitalSpinBox;
    QPushButton* m_backtestButton;
    QChartView* m_backtestChartView;
    QChart* m_backtestChart;
    QTableWidget* m_backtestResults;

    // Portfolio Tab
    QWidget* m_portfolioTab;
    QTextEdit* m_symbolsEdit;
    QComboBox* m_optimizationCombo;
    QPushButton* m_optimizeButton;
    QTableWidget* m_portfolioTable;
    QChartView* m_portfolioChartView;
    QChart* m_portfolioChart;

    // Risk Tab
    QWidget* m_riskTab;
    QDoubleSpinBox* m_confidenceLevelSpinBox;
    QSpinBox* m_varWindowSpinBox;
    QTableWidget* m_riskTable;
    QChartView* m_riskChartView;
    QChart* m_riskChart;

    // Charts Tab
    QWidget* m_chartsTab;
    QChartView* m_priceChartView;
    QChart* m_priceChart;
    QChartView* m_volumeChartView;
    QChart* m_volumeChart;
    QChartView* m_indicatorsChartView;
    QChart* m_indicatorsChart;

    // Settings Tab
    QWidget* m_settingsTab;
    QComboBox* m_themeCombo;
    QSpinBox* m_threadsSpinBox;
    QComboBox* m_deviceCombo;
    QPushButton* m_applySettingsButton;
    QPushButton* m_resetSettingsButton;

    // Status and progress
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;

    // Menus and actions
    QMenuBar* m_menuBar;
    QToolBar* m_toolBar;
    QAction* m_newAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_exitAction;
    QAction* m_preferencesAction;
    QAction* m_aboutAction;

    // Backend components
    std::unique_ptr<stock_predict::IStockPredictor> m_predictor;
    std::unique_ptr<stock_predict::DataManager> m_dataManager;
    stock_predict::PredictorConfig m_config;
    std::vector<stock_predict::MarketData> m_currentData;

    // Threading
    QThread* m_workerThread;
    PredictionWorker* m_worker;
    QThread* m_realtimeThread;
    RealTimeUpdater* m_realtimeUpdater;
    QTimer* m_updateTimer;

    // Settings
    QSettings* m_settings;

    // State
    bool m_isRealTimeMode;
    bool m_isTraining;
    QString m_currentSymbol;
    QString m_projectFile;
};

/**
 * @brief Worker thread for model training and predictions
 */
class PredictionWorker : public QObject {
    Q_OBJECT

   public:
    explicit PredictionWorker(QObject* parent = nullptr);

   public slots:
    void trainModel(const std::vector<stock_predict::MarketData>& data,
                    const stock_predict::PredictorConfig& config, int epochs);
    void makePrediction(const std::vector<stock_predict::MarketData>& data,
                        const stock_predict::PredictorConfig& config, int days);
    void runBacktest(const std::vector<stock_predict::MarketData>& data,
                     const stock_predict::PredictorConfig& config, double initialCapital);

   signals:
    void trainingProgress(int percentage);
    void trainingComplete(bool success, const QString& message);
    void predictionReady(const stock_predict::PredictionResult& result);
    void backtestComplete(const QVariantMap& results);
    void errorOccurred(const QString& error);

   private:
    std::unique_ptr<stock_predict::IStockPredictor> m_predictor;
    QMutex m_mutex;
};

/**
 * @brief Real-time data updater
 */
class RealTimeUpdater : public QObject {
    Q_OBJECT

   public:
    explicit RealTimeUpdater(QObject* parent = nullptr);

    void setSymbol(const QString& symbol);
    void setUpdateInterval(int milliseconds);

   public slots:
    void start();
    void stop();

   signals:
    void dataUpdated(const stock_predict::MarketData& data);
    void errorOccurred(const QString& error);

   private slots:
    void updateData();

   private:
    QString m_symbol;
    QTimer* m_timer;
    std::unique_ptr<stock_predict::DataManager> m_dataManager;
    QMutex m_mutex;
    bool m_isRunning;
};
