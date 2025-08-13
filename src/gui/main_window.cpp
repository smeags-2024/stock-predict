#ifdef QT_AVAILABLE
#include "stock_predict/gui/main_window.hpp"
#include <QApplication>
#include <QColorDialog>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFontDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSplitterHandle>
#include <QStandardPaths>
#include <algorithm>
#include <cmath>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_centralTabs(nullptr),
      m_predictor(nullptr),
      m_dataManager(std::make_unique<stock_predict::DataManager>()),
      m_workerThread(nullptr),
      m_worker(nullptr),
      m_realtimeThread(nullptr),
      m_realtimeUpdater(nullptr),
      m_updateTimer(new QTimer(this)),
      m_settings(new QSettings("StockPredict", "StockPredictGUI", this)),
      m_isRealTimeMode(false),
      m_isTraining(false),
      m_currentSymbol("AAPL"),
      m_projectFile("") {
    setupUI();
    setupConnections();
    loadSettings();

    // Initialize default configuration
    m_config.model_type = "ensemble";
    m_config.sequence_length = 60;
    m_config.prediction_horizon = 1;
    m_config.use_gpu = false;

    setWindowTitle("Stock Predict - Advanced C++ Trading System");
    setWindowIcon(QIcon(":/icons/app_icon.png"));
    resize(1400, 900);

    updateStatusBar("Ready");
}

MainWindow::~MainWindow() {
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }

    if (m_realtimeThread && m_realtimeThread->isRunning()) {
        m_realtimeThread->quit();
        m_realtimeThread->wait();
    }

    saveSettings();
}

void MainWindow::setupUI() {
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupTabs();
}

void MainWindow::setupMenuBar() {
    // File menu
    QMenu* fileMenu = menuBar()->addMenu("&File");

    m_newAction = new QAction("&New Project", this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip("Create a new project");
    fileMenu->addAction(m_newAction);

    m_openAction = new QAction("&Open Project", this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip("Open an existing project");
    fileMenu->addAction(m_openAction);

    fileMenu->addSeparator();

    m_saveAction = new QAction("&Save Project", this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip("Save the current project");
    fileMenu->addAction(m_saveAction);

    m_saveAsAction = new QAction("Save Project &As...", this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip("Save the current project with a new name");
    fileMenu->addAction(m_saveAsAction);

    fileMenu->addSeparator();

    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    fileMenu->addAction(m_exitAction);

    // Tools menu
    QMenu* toolsMenu = menuBar()->addMenu("&Tools");

    m_preferencesAction = new QAction("&Preferences", this);
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    m_preferencesAction->setStatusTip("Configure application settings");
    toolsMenu->addAction(m_preferencesAction);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");

    m_aboutAction = new QAction("&About", this);
    m_aboutAction->setStatusTip("Show information about the application");
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupToolBar() {
    m_toolBar = addToolBar("Main");
    m_toolBar->addAction(m_newAction);
    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addSeparator();

    // Quick action buttons
    QPushButton* quickPredictBtn = new QPushButton("Quick Predict", this);
    quickPredictBtn->setStatusTip("Make a quick prediction with current settings");
    m_toolBar->addWidget(quickPredictBtn);
    connect(quickPredictBtn, &QPushButton::clicked, this, &MainWindow::makePrediction);

    QPushButton* realTimeBtn = new QPushButton("Real-Time", this);
    realTimeBtn->setStatusTip("Toggle real-time mode");
    realTimeBtn->setCheckable(true);
    m_toolBar->addWidget(realTimeBtn);
    connect(realTimeBtn, &QPushButton::toggled, [this](bool checked) {
        if (checked) {
            startRealTimeMode();
        } else {
            stopRealTimeMode();
        }
    });
}

void MainWindow::setupStatusBar() {
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    statusBar()->addPermanentWidget(m_progressBar);

    QLabel* versionLabel = new QLabel("v1.0.0", this);
    statusBar()->addPermanentWidget(versionLabel);
}

void MainWindow::setupTabs() {
    m_centralTabs = new QTabWidget(this);
    setCentralWidget(m_centralTabs);

    setupPredictionTab();
    setupTrainingTab();
    setupBacktestTab();
    setupPortfolioTab();
    setupRiskTab();
    setupChartsTab();
    setupSettingsTab();
}

void MainWindow::setupPredictionTab() {
    m_predictionTab = new QWidget();
    m_centralTabs->addTab(m_predictionTab, "Prediction");

    QHBoxLayout* mainLayout = new QHBoxLayout(m_predictionTab);

    // Left panel - Controls
    QWidget* controlPanel = new QWidget();
    controlPanel->setMaximumWidth(300);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    // Input group
    QGroupBox* inputGroup = new QGroupBox("Input Parameters");
    QGridLayout* inputLayout = new QGridLayout(inputGroup);

    inputLayout->addWidget(new QLabel("Symbol:"), 0, 0);
    m_symbolEdit = new QLineEdit("AAPL");
    inputLayout->addWidget(m_symbolEdit, 0, 1);

    inputLayout->addWidget(new QLabel("Model:"), 1, 0);
    m_modelCombo = new QComboBox();
    m_modelCombo->addItems({"LSTM", "Transformer", "Ensemble"});
    m_modelCombo->setCurrentText("Ensemble");
    inputLayout->addWidget(m_modelCombo, 1, 1);

    inputLayout->addWidget(new QLabel("Days:"), 2, 0);
    m_predictionDaysSpinBox = new QSpinBox();
    m_predictionDaysSpinBox->setRange(1, 30);
    m_predictionDaysSpinBox->setValue(5);
    inputLayout->addWidget(m_predictionDaysSpinBox, 2, 1);

    controlLayout->addWidget(inputGroup);

    // Action buttons
    QGroupBox* actionGroup = new QGroupBox("Actions");
    QVBoxLayout* actionLayout = new QVBoxLayout(actionGroup);

    m_predictButton = new QPushButton("Make Prediction");
    m_predictButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; "
        "}");
    actionLayout->addWidget(m_predictButton);

    m_realTimeButton = new QPushButton("Start Real-Time");
    m_realTimeButton->setCheckable(true);
    m_realTimeButton->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; "
        "}");
    actionLayout->addWidget(m_realTimeButton);

    controlLayout->addWidget(actionGroup);

    // Results group
    QGroupBox* resultsGroup = new QGroupBox("Current Results");
    QGridLayout* resultsLayout = new QGridLayout(resultsGroup);

    resultsLayout->addWidget(new QLabel("Current Price:"), 0, 0);
    m_currentPriceLabel = new QLabel("--");
    m_currentPriceLabel->setStyleSheet("font-weight: bold; color: #333;");
    resultsLayout->addWidget(m_currentPriceLabel, 0, 1);

    resultsLayout->addWidget(new QLabel("Predicted Price:"), 1, 0);
    m_predictedPriceLabel = new QLabel("--");
    m_predictedPriceLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
    resultsLayout->addWidget(m_predictedPriceLabel, 1, 1);

    resultsLayout->addWidget(new QLabel("Confidence:"), 2, 0);
    m_confidenceLabel = new QLabel("--");
    m_confidenceLabel->setStyleSheet("font-weight: bold; color: #FF9800;");
    resultsLayout->addWidget(m_confidenceLabel, 2, 1);

    resultsLayout->addWidget(new QLabel("Expected Return:"), 3, 0);
    m_returnLabel = new QLabel("--");
    m_returnLabel->setStyleSheet("font-weight: bold; color: #2196F3;");
    resultsLayout->addWidget(m_returnLabel, 3, 1);

    controlLayout->addWidget(resultsGroup);

    controlLayout->addStretch();
    mainLayout->addWidget(controlPanel);

    // Right panel - Chart and table
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical);

    // Chart
    m_predictionChart = new QChart();
    m_predictionChart->setTitle("Price Prediction Chart");
    m_predictionChart->setAnimationOptions(QChart::SeriesAnimations);
    m_predictionChartView = new QChartView(m_predictionChart);
    m_predictionChartView->setRenderHint(QPainter::Antialiasing);
    rightSplitter->addWidget(m_predictionChartView);

    // Table
    m_predictionTable = new QTableWidget();
    m_predictionTable->setColumnCount(6);
    m_predictionTable->setHorizontalHeaderLabels(
        {"Day", "Date", "Price", "Return", "Confidence", "Risk"});
    m_predictionTable->horizontalHeader()->setStretchLastSection(true);
    m_predictionTable->setAlternatingRowColors(true);
    rightSplitter->addWidget(m_predictionTable);

    rightSplitter->setSizes({400, 200});
    mainLayout->addWidget(rightSplitter);
}

void MainWindow::setupTrainingTab() {
    m_trainingTab = new QWidget();
    m_centralTabs->addTab(m_trainingTab, "Training");

    QHBoxLayout* mainLayout = new QHBoxLayout(m_trainingTab);

    // Left panel - Training controls
    QWidget* controlPanel = new QWidget();
    controlPanel->setMaximumWidth(350);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    // Data source group
    QGroupBox* dataGroup = new QGroupBox("Data Source");
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);

    QHBoxLayout* sourceLayout = new QHBoxLayout();
    m_dataSourceEdit = new QLineEdit("data/sample_data.csv");
    QPushButton* browseButton = new QPushButton("Browse...");
    sourceLayout->addWidget(m_dataSourceEdit);
    sourceLayout->addWidget(browseButton);
    dataLayout->addLayout(sourceLayout);

    m_loadDataButton = new QPushButton("Load Data");
    m_loadDataButton->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; "
        "}");
    dataLayout->addWidget(m_loadDataButton);

    controlLayout->addWidget(dataGroup);

    // Training parameters group
    QGroupBox* paramsGroup = new QGroupBox("Training Parameters");
    QGridLayout* paramsLayout = new QGridLayout(paramsGroup);

    paramsLayout->addWidget(new QLabel("Epochs:"), 0, 0);
    m_epochsSpinBox = new QSpinBox();
    m_epochsSpinBox->setRange(1, 1000);
    m_epochsSpinBox->setValue(100);
    paramsLayout->addWidget(m_epochsSpinBox, 0, 1);

    paramsLayout->addWidget(new QLabel("Learning Rate:"), 1, 0);
    m_learningRateSpinBox = new QDoubleSpinBox();
    m_learningRateSpinBox->setRange(0.0001, 0.1);
    m_learningRateSpinBox->setValue(0.001);
    m_learningRateSpinBox->setDecimals(4);
    m_learningRateSpinBox->setSingleStep(0.0001);
    paramsLayout->addWidget(m_learningRateSpinBox, 1, 1);

    controlLayout->addWidget(paramsGroup);

    // Training control
    m_trainButton = new QPushButton("Start Training");
    m_trainButton->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 10px; "
        "}");
    controlLayout->addWidget(m_trainButton);

    m_trainingProgress = new QProgressBar();
    controlLayout->addWidget(m_trainingProgress);

    // Metrics table
    QGroupBox* metricsGroup = new QGroupBox("Model Metrics");
    QVBoxLayout* metricsLayout = new QVBoxLayout(metricsGroup);

    m_metricsTable = new QTableWidget();
    m_metricsTable->setColumnCount(2);
    m_metricsTable->setHorizontalHeaderLabels({"Metric", "Value"});
    m_metricsTable->horizontalHeader()->setStretchLastSection(true);
    metricsLayout->addWidget(m_metricsTable);

    controlLayout->addWidget(metricsGroup);
    controlLayout->addStretch();

    mainLayout->addWidget(controlPanel);

    // Right panel - Training log
    QGroupBox* logGroup = new QGroupBox("Training Log");
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);

    m_trainingLog = new QTextEdit();
    m_trainingLog->setFont(QFont("Consolas", 9));
    m_trainingLog->setReadOnly(true);
    logLayout->addWidget(m_trainingLog);

    mainLayout->addWidget(logGroup);

    // Connect browse button
    connect(browseButton, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getOpenFileName(
            this, "Open Data File",
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            "CSV Files (*.csv);;All Files (*)");
        if (!fileName.isEmpty()) {
            m_dataSourceEdit->setText(fileName);
        }
    });
}

void MainWindow::setupBacktestTab() {
    m_backtestTab = new QWidget();
    m_centralTabs->addTab(m_backtestTab, "Backtest");

    QHBoxLayout* mainLayout = new QHBoxLayout(m_backtestTab);

    // Left panel - Backtest controls
    QWidget* controlPanel = new QWidget();
    controlPanel->setMaximumWidth(300);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    // Parameters group
    QGroupBox* paramsGroup = new QGroupBox("Backtest Parameters");
    QGridLayout* paramsLayout = new QGridLayout(paramsGroup);

    paramsLayout->addWidget(new QLabel("Start Date:"), 0, 0);
    m_startDateEdit = new QLineEdit("2020-01-01");
    paramsLayout->addWidget(m_startDateEdit, 0, 1);

    paramsLayout->addWidget(new QLabel("End Date:"), 1, 0);
    m_endDateEdit = new QLineEdit("2023-12-31");
    paramsLayout->addWidget(m_endDateEdit, 1, 1);

    paramsLayout->addWidget(new QLabel("Initial Capital:"), 2, 0);
    m_initialCapitalSpinBox = new QDoubleSpinBox();
    m_initialCapitalSpinBox->setRange(1000, 10000000);
    m_initialCapitalSpinBox->setValue(100000);
    m_initialCapitalSpinBox->setPrefix("$");
    paramsLayout->addWidget(m_initialCapitalSpinBox, 2, 1);

    controlLayout->addWidget(paramsGroup);

    m_backtestButton = new QPushButton("Run Backtest");
    m_backtestButton->setStyleSheet(
        "QPushButton { background-color: #FF9800; color: white; font-weight: bold; padding: 10px; "
        "}");
    controlLayout->addWidget(m_backtestButton);

    // Results table
    QGroupBox* resultsGroup = new QGroupBox("Backtest Results");
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroup);

    m_backtestResults = new QTableWidget();
    m_backtestResults->setColumnCount(2);
    m_backtestResults->setHorizontalHeaderLabels({"Metric", "Value"});
    m_backtestResults->horizontalHeader()->setStretchLastSection(true);
    resultsLayout->addWidget(m_backtestResults);

    controlLayout->addWidget(resultsGroup);
    controlLayout->addStretch();

    mainLayout->addWidget(controlPanel);

    // Right panel - Backtest chart
    m_backtestChart = new QChart();
    m_backtestChart->setTitle("Backtest Performance");
    m_backtestChart->setAnimationOptions(QChart::SeriesAnimations);
    m_backtestChartView = new QChartView(m_backtestChart);
    m_backtestChartView->setRenderHint(QPainter::Antialiasing);

    mainLayout->addWidget(m_backtestChartView);
}

void MainWindow::setupPortfolioTab() {
    m_portfolioTab = new QWidget();
    m_centralTabs->addTab(m_portfolioTab, "Portfolio");

    QHBoxLayout* mainLayout = new QHBoxLayout(m_portfolioTab);

    // Left panel - Portfolio controls
    QWidget* controlPanel = new QWidget();
    controlPanel->setMaximumWidth(300);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    // Symbols group
    QGroupBox* symbolsGroup = new QGroupBox("Assets");
    QVBoxLayout* symbolsLayout = new QVBoxLayout(symbolsGroup);

    QLabel* symbolsLabel = new QLabel("Symbols (one per line):");
    symbolsLayout->addWidget(symbolsLabel);

    m_symbolsEdit = new QTextEdit();
    m_symbolsEdit->setMaximumHeight(100);
    m_symbolsEdit->setPlainText("AAPL\nGOOGL\nMSFT\nAMZN\nTSLA");
    symbolsLayout->addWidget(m_symbolsEdit);

    controlLayout->addWidget(symbolsGroup);

    // Optimization group
    QGroupBox* optimGroup = new QGroupBox("Optimization");
    QGridLayout* optimLayout = new QGridLayout(optimGroup);

    optimLayout->addWidget(new QLabel("Method:"), 0, 0);
    m_optimizationCombo = new QComboBox();
    m_optimizationCombo->addItems(
        {"Mean Variance", "Risk Parity", "Minimum Variance", "Maximum Sharpe"});
    optimLayout->addWidget(m_optimizationCombo, 0, 1);

    controlLayout->addWidget(optimGroup);

    m_optimizeButton = new QPushButton("Optimize Portfolio");
    m_optimizeButton->setStyleSheet(
        "QPushButton { background-color: #9C27B0; color: white; font-weight: bold; padding: 10px; "
        "}");
    controlLayout->addWidget(m_optimizeButton);

    // Portfolio weights table
    QGroupBox* weightsGroup = new QGroupBox("Optimal Weights");
    QVBoxLayout* weightsLayout = new QVBoxLayout(weightsGroup);

    m_portfolioTable = new QTableWidget();
    m_portfolioTable->setColumnCount(3);
    m_portfolioTable->setHorizontalHeaderLabels({"Symbol", "Weight", "Allocation"});
    m_portfolioTable->horizontalHeader()->setStretchLastSection(true);
    weightsLayout->addWidget(m_portfolioTable);

    controlLayout->addWidget(weightsGroup);
    controlLayout->addStretch();

    mainLayout->addWidget(controlPanel);

    // Right panel - Portfolio chart
    m_portfolioChart = new QChart();
    m_portfolioChart->setTitle("Portfolio Allocation");
    m_portfolioChart->setAnimationOptions(QChart::SeriesAnimations);
    m_portfolioChartView = new QChartView(m_portfolioChart);
    m_portfolioChartView->setRenderHint(QPainter::Antialiasing);

    mainLayout->addWidget(m_portfolioChartView);
}

void MainWindow::setupRiskTab() {
    m_riskTab = new QWidget();
    m_centralTabs->addTab(m_riskTab, "Risk Analysis");

    QHBoxLayout* mainLayout = new QHBoxLayout(m_riskTab);

    // Left panel - Risk controls
    QWidget* controlPanel = new QWidget();
    controlPanel->setMaximumWidth(300);
    QVBoxLayout* controlLayout = new QVBoxLayout(controlPanel);

    // Risk parameters group
    QGroupBox* paramsGroup = new QGroupBox("Risk Parameters");
    QGridLayout* paramsLayout = new QGridLayout(paramsGroup);

    paramsLayout->addWidget(new QLabel("Confidence Level:"), 0, 0);
    m_confidenceLevelSpinBox = new QDoubleSpinBox();
    m_confidenceLevelSpinBox->setRange(0.90, 0.99);
    m_confidenceLevelSpinBox->setValue(0.95);
    m_confidenceLevelSpinBox->setSingleStep(0.01);
    m_confidenceLevelSpinBox->setSuffix("%");
    paramsLayout->addWidget(m_confidenceLevelSpinBox, 0, 1);

    paramsLayout->addWidget(new QLabel("VaR Window:"), 1, 0);
    m_varWindowSpinBox = new QSpinBox();
    m_varWindowSpinBox->setRange(30, 1000);
    m_varWindowSpinBox->setValue(252);
    m_varWindowSpinBox->setSuffix(" days");
    paramsLayout->addWidget(m_varWindowSpinBox, 1, 1);

    controlLayout->addWidget(paramsGroup);

    // Risk metrics table
    QGroupBox* metricsGroup = new QGroupBox("Risk Metrics");
    QVBoxLayout* metricsLayout = new QVBoxLayout(metricsGroup);

    m_riskTable = new QTableWidget();
    m_riskTable->setColumnCount(2);
    m_riskTable->setHorizontalHeaderLabels({"Metric", "Value"});
    m_riskTable->horizontalHeader()->setStretchLastSection(true);
    metricsLayout->addWidget(m_riskTable);

    controlLayout->addWidget(metricsGroup);
    controlLayout->addStretch();

    mainLayout->addWidget(controlPanel);

    // Right panel - Risk chart
    m_riskChart = new QChart();
    m_riskChart->setTitle("Risk Analysis");
    m_riskChart->setAnimationOptions(QChart::SeriesAnimations);
    m_riskChartView = new QChartView(m_riskChart);
    m_riskChartView->setRenderHint(QPainter::Antialiasing);

    mainLayout->addWidget(m_riskChartView);
}

void MainWindow::setupChartsTab() {
    m_chartsTab = new QWidget();
    m_centralTabs->addTab(m_chartsTab, "Charts");

    QVBoxLayout* mainLayout = new QVBoxLayout(m_chartsTab);

    // Top row - Price and Volume
    QHBoxLayout* topRow = new QHBoxLayout();

    m_priceChart = new QChart();
    m_priceChart->setTitle("Price Chart");
    m_priceChart->setAnimationOptions(QChart::SeriesAnimations);
    m_priceChartView = new QChartView(m_priceChart);
    m_priceChartView->setRenderHint(QPainter::Antialiasing);
    topRow->addWidget(m_priceChartView);

    m_volumeChart = new QChart();
    m_volumeChart->setTitle("Volume Chart");
    m_volumeChart->setAnimationOptions(QChart::SeriesAnimations);
    m_volumeChartView = new QChartView(m_volumeChart);
    m_volumeChartView->setRenderHint(QPainter::Antialiasing);
    topRow->addWidget(m_volumeChartView);

    mainLayout->addLayout(topRow);

    // Bottom row - Technical indicators
    m_indicatorsChart = new QChart();
    m_indicatorsChart->setTitle("Technical Indicators");
    m_indicatorsChart->setAnimationOptions(QChart::SeriesAnimations);
    m_indicatorsChartView = new QChartView(m_indicatorsChart);
    m_indicatorsChartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_indicatorsChartView);
}

void MainWindow::setupSettingsTab() {
    m_settingsTab = new QWidget();
    m_centralTabs->addTab(m_settingsTab, "Settings");

    QHBoxLayout* mainLayout = new QHBoxLayout(m_settingsTab);

    // Settings form
    QGroupBox* settingsGroup = new QGroupBox("Application Settings");
    QGridLayout* settingsLayout = new QGridLayout(settingsGroup);

    settingsLayout->addWidget(new QLabel("Theme:"), 0, 0);
    m_themeCombo = new QComboBox();
    m_themeCombo->addItems({"Light", "Dark", "Auto"});
    settingsLayout->addWidget(m_themeCombo, 0, 1);

    settingsLayout->addWidget(new QLabel("CPU Threads:"), 1, 0);
    m_threadsSpinBox = new QSpinBox();
    m_threadsSpinBox->setRange(1, QThread::idealThreadCount());
    m_threadsSpinBox->setValue(QThread::idealThreadCount());
    settingsLayout->addWidget(m_threadsSpinBox, 1, 1);

    settingsLayout->addWidget(new QLabel("Compute Device:"), 2, 0);
    m_deviceCombo = new QComboBox();
    m_deviceCombo->addItems({"CPU", "GPU (CUDA)", "GPU (OpenCL)"});
    settingsLayout->addWidget(m_deviceCombo, 2, 1);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_applySettingsButton = new QPushButton("Apply Settings");
    m_resetSettingsButton = new QPushButton("Reset to Defaults");
    buttonLayout->addWidget(m_applySettingsButton);
    buttonLayout->addWidget(m_resetSettingsButton);
    buttonLayout->addStretch();

    settingsLayout->addLayout(buttonLayout, 3, 0, 1, 2);

    mainLayout->addWidget(settingsGroup);
    mainLayout->addStretch();
}

void MainWindow::setupConnections() {
    // Menu actions
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newProject);
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openProject);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveProject);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveProjectAs);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    // Prediction tab
    connect(m_predictButton, &QPushButton::clicked, this, &MainWindow::makePrediction);
    connect(m_realTimeButton, &QPushButton::toggled, [this](bool checked) {
        if (checked) {
            startRealTimeMode();
        } else {
            stopRealTimeMode();
        }
    });

    // Training tab
    connect(m_loadDataButton, &QPushButton::clicked, this, &MainWindow::loadData);
    connect(m_trainButton, &QPushButton::clicked, this, &MainWindow::trainModel);

    // Backtest tab
    connect(m_backtestButton, &QPushButton::clicked, this, &MainWindow::runBacktest);

    // Portfolio tab
    connect(m_optimizeButton, &QPushButton::clicked, this, &MainWindow::optimizePortfolio);

    // Settings tab
    connect(m_applySettingsButton, &QPushButton::clicked, this, &MainWindow::applySettings);
    connect(m_resetSettingsButton, &QPushButton::clicked, this, &MainWindow::resetSettings);

    // Update timer
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateModelMetrics);
}

// Placeholder implementations for slot functions
void MainWindow::newProject() { updateStatusBar("New project created"); }

void MainWindow::openProject() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Project", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Stock Predict Projects (*.spp);;All Files (*)");
    if (!fileName.isEmpty()) {
        m_projectFile = fileName;
        updateStatusBar("Project opened: " + QFileInfo(fileName).baseName());
    }
}

void MainWindow::saveProject() {
    if (m_projectFile.isEmpty()) {
        saveProjectAs();
    } else {
        updateStatusBar("Project saved");
    }
}

void MainWindow::saveProjectAs() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Project", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Stock Predict Projects (*.spp);;All Files (*)");
    if (!fileName.isEmpty()) {
        m_projectFile = fileName;
        updateStatusBar("Project saved as: " + QFileInfo(fileName).baseName());
    }
}

void MainWindow::showPreferences() { m_centralTabs->setCurrentWidget(m_settingsTab); }

void MainWindow::showAbout() {
    QMessageBox::about(this, "About Stock Predict",
                       "<h2>Stock Predict v1.0.0</h2>"
                       "<p>Advanced C++ Stock Price Prediction System</p>"
                       "<p>Features:</p>"
                       "<ul>"
                       "<li>High-performance LSTM and Transformer models</li>"
                       "<li>Real-time prediction capabilities</li>"
                       "<li>Comprehensive risk management</li>"
                       "<li>Portfolio optimization</li>"
                       "<li>Professional backtesting</li>"
                       "</ul>"
                       "<p><b>Copyright © 2024 Stock Predict Team</b></p>");
}

void MainWindow::loadData() {
    updateStatusBar("Loading data...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);  // Indeterminate progress

    // Simulate data loading (replace with actual implementation)
    QTimer::singleShot(2000, [this]() {
        m_progressBar->setVisible(false);
        updateStatusBar("Data loaded successfully");
        m_trainingLog->append("Data loaded from: " + m_dataSourceEdit->text());
        m_trainingLog->append("Records: 1000, Features: 8");
    });
}

void MainWindow::trainModel() {
    if (m_isTraining) {
        return;
    }

    m_isTraining = true;
    m_trainButton->setText("Training...");
    m_trainButton->setEnabled(false);

    updateStatusBar("Training model...");
    m_trainingProgress->setVisible(true);
    m_trainingProgress->setRange(0, m_epochsSpinBox->value());
    m_trainingProgress->setValue(0);

    m_trainingLog->append("=== Training Started ===");
    m_trainingLog->append(QString("Model: %1").arg(m_modelCombo->currentText()));
    m_trainingLog->append(QString("Epochs: %1").arg(m_epochsSpinBox->value()));
    m_trainingLog->append(QString("Learning Rate: %1").arg(m_learningRateSpinBox->value()));

    // Simulate training progress
    QTimer* progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, [this, progressTimer]() {
        int current = m_trainingProgress->value();
        if (current < m_trainingProgress->maximum()) {
            m_trainingProgress->setValue(current + 1);
            m_trainingLog->append(QString("Epoch %1/%2 - Loss: %3")
                                      .arg(current + 1)
                                      .arg(m_trainingProgress->maximum())
                                      .arg(0.1 - (current * 0.001), 0, 'f', 4));
        } else {
            progressTimer->stop();
            progressTimer->deleteLater();

            m_isTraining = false;
            m_trainButton->setText("Start Training");
            m_trainButton->setEnabled(true);
            m_trainingProgress->setVisible(false);

            m_trainingLog->append("=== Training Completed ===");
            updateStatusBar("Model training completed");
            updateModelMetrics();
        }
    });
    progressTimer->start(100);  // Update every 100ms for demo
}

void MainWindow::makePrediction() {
    if (!m_predictor) {
        // Create predictor if not exists
        m_config.model_type = m_modelCombo->currentText().toLower().toStdString();
        m_predictor = stock_predict::create_predictor(m_symbolEdit->text().toStdString(), m_config);
    }

    updateStatusBar("Making prediction...");

    // Simulate prediction (replace with actual implementation)
    QTimer::singleShot(500, [this]() {
        // Generate sample prediction results
        double currentPrice = 150.50;
        double predictedPrice = currentPrice * (1.0 + (rand() % 100 - 50) / 1000.0);
        double confidence = 0.75 + (rand() % 25) / 100.0;
        double expectedReturn = (predictedPrice - currentPrice) / currentPrice;

        m_currentPriceLabel->setText(formatCurrency(currentPrice));
        m_predictedPriceLabel->setText(formatCurrency(predictedPrice));
        m_confidenceLabel->setText(formatPercentage(confidence));
        m_returnLabel->setText(formatPercentage(expectedReturn));

        // Update prediction table
        int days = m_predictionDaysSpinBox->value();
        m_predictionTable->setRowCount(days);

        for (int i = 0; i < days; ++i) {
            double dayPrice = predictedPrice * (1.0 + i * 0.002);
            double dayReturn = (dayPrice - currentPrice) / currentPrice;
            double dayConfidence = confidence * (1.0 - i * 0.05);
            double dayRisk = std::abs(dayReturn) * 0.1;

            m_predictionTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
            m_predictionTable->setItem(
                i, 1, new QTableWidgetItem(QDate::currentDate().addDays(i + 1).toString()));
            m_predictionTable->setItem(i, 2, new QTableWidgetItem(formatCurrency(dayPrice)));
            m_predictionTable->setItem(i, 3, new QTableWidgetItem(formatPercentage(dayReturn)));
            m_predictionTable->setItem(i, 4, new QTableWidgetItem(formatPercentage(dayConfidence)));
            m_predictionTable->setItem(i, 5, new QTableWidgetItem(formatPercentage(dayRisk)));
        }

        updateStatusBar("Prediction completed");
        refreshCharts();
    });
}

void MainWindow::startRealTimeMode() {
    if (m_isRealTimeMode) {
        return;
    }

    m_isRealTimeMode = true;
    m_realTimeButton->setText("Stop Real-Time");
    m_realTimeButton->setStyleSheet(
        "QPushButton { background-color: #F44336; color: white; font-weight: bold; padding: 8px; "
        "}");

    updateStatusBar("Real-time mode started");

    // Start update timer for demo
    m_updateTimer->start(5000);  // Update every 5 seconds
}

void MainWindow::stopRealTimeMode() {
    if (!m_isRealTimeMode) {
        return;
    }

    m_isRealTimeMode = false;
    m_realTimeButton->setText("Start Real-Time");
    m_realTimeButton->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px; "
        "}");

    m_updateTimer->stop();
    updateStatusBar("Real-time mode stopped");
}

void MainWindow::runBacktest() {
    updateStatusBar("Running backtest...");

    // Simulate backtest
    QTimer::singleShot(3000, [this]() {
        // Update backtest results table
        QStringList metrics = {"Total Return", "Sharpe Ratio", "Max Drawdown", "Win Rate",
                               "Profit Factor"};
        QStringList values = {"15.6%", "1.23", "-8.4%", "62.5%", "1.85"};

        m_backtestResults->setRowCount(metrics.size());
        for (int i = 0; i < metrics.size(); ++i) {
            m_backtestResults->setItem(i, 0, new QTableWidgetItem(metrics[i]));
            m_backtestResults->setItem(i, 1, new QTableWidgetItem(values[i]));
        }

        updateStatusBar("Backtest completed");
    });
}

void MainWindow::optimizePortfolio() {
    updateStatusBar("Optimizing portfolio...");

    // Simulate portfolio optimization
    QTimer::singleShot(2000, [this]() {
        QStringList symbols = m_symbolsEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
        m_portfolioTable->setRowCount(symbols.size());

        double totalWeight = 0.0;
        QList<double> weights;

        // Generate random weights
        for (int i = 0; i < symbols.size(); ++i) {
            double weight = (rand() % 100) / 100.0;
            weights.append(weight);
            totalWeight += weight;
        }

        // Normalize weights
        double capital = 100000.0;
        for (int i = 0; i < symbols.size(); ++i) {
            double normalizedWeight = weights[i] / totalWeight;
            double allocation = capital * normalizedWeight;

            m_portfolioTable->setItem(i, 0, new QTableWidgetItem(symbols[i]));
            m_portfolioTable->setItem(i, 1,
                                      new QTableWidgetItem(formatPercentage(normalizedWeight)));
            m_portfolioTable->setItem(i, 2, new QTableWidgetItem(formatCurrency(allocation)));
        }

        updateStatusBar("Portfolio optimization completed");
    });
}

// Helper methods
void MainWindow::onDataLoaded(const std::vector<stock_predict::MarketData>& data) {
    m_currentData = data;
    updateStatusBar(QString("Loaded %1 data points").arg(data.size()));
}

void MainWindow::onModelTrained(bool success, const QString& message) {
    if (success) {
        updateStatusBar("Model training completed successfully");
    } else {
        updateStatusBar("Model training failed: " + message);
    }
}

void MainWindow::onPredictionReady(const stock_predict::PredictionResult& result) {
    m_predictedPriceLabel->setText(formatCurrency(result.price));
    m_confidenceLabel->setText(formatPercentage(result.confidence));
    m_returnLabel->setText(formatPercentage(result.expected_return));
}

void MainWindow::onRealTimeUpdate(const stock_predict::MarketData& data) {
    m_currentPriceLabel->setText(formatCurrency(data.close));
    // Trigger new prediction with updated data
    if (m_isRealTimeMode) {
        makePrediction();
    }
}

void MainWindow::onBacktestComplete(const QVariantMap& results) {
    // Update backtest results display
    updateStatusBar("Backtest analysis completed");
}

void MainWindow::updateStatusBar(const QString& message) {
    m_statusLabel->setText(message);
    statusBar()->showMessage(message, 5000);
}

void MainWindow::updateProgress(int value) { m_progressBar->setValue(value); }

void MainWindow::updateModelMetrics() {
    if (!m_predictor) {
        return;
    }

    // Get metrics from predictor
    auto metrics = m_predictor->get_performance_metrics();

    m_metricsTable->setRowCount(static_cast<int>(metrics.size()));
    for (size_t i = 0; i < metrics.size(); ++i) {
        m_metricsTable->setItem(static_cast<int>(i), 0,
                                new QTableWidgetItem(QString::fromStdString(metrics[i].first)));
        m_metricsTable->setItem(static_cast<int>(i), 1,
                                new QTableWidgetItem(QString::number(metrics[i].second, 'f', 4)));
    }
}

void MainWindow::updateRiskMetrics() {
    // Calculate and display risk metrics
    QStringList riskMetrics = {"VaR (95%)", "Expected Shortfall", "Volatility", "Sharpe Ratio",
                               "Max Drawdown"};
    QStringList riskValues = {"-2.1%", "-3.2%", "18.5%", "1.23", "-8.4%"};

    m_riskTable->setRowCount(riskMetrics.size());
    for (int i = 0; i < riskMetrics.size(); ++i) {
        m_riskTable->setItem(i, 0, new QTableWidgetItem(riskMetrics[i]));
        m_riskTable->setItem(i, 1, new QTableWidgetItem(riskValues[i]));
    }
}

void MainWindow::refreshCharts() {
    // Update all charts with current data
    // Implementation would update the actual chart series
}

void MainWindow::applySettings() {
    // Apply settings from UI to application
    m_config.use_gpu = (m_deviceCombo->currentText().contains("GPU"));
    m_config.num_threads = m_threadsSpinBox->value();

    updateStatusBar("Settings applied");
}

void MainWindow::resetSettings() {
    // Reset all settings to defaults
    m_themeCombo->setCurrentText("Light");
    m_threadsSpinBox->setValue(QThread::idealThreadCount());
    m_deviceCombo->setCurrentText("CPU");

    updateStatusBar("Settings reset to defaults");
}

void MainWindow::loadSettings() {
    // Load settings from QSettings
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());

    m_symbolEdit->setText(m_settings->value("symbol", "AAPL").toString());
    m_modelCombo->setCurrentText(m_settings->value("model", "Ensemble").toString());
}

void MainWindow::saveSettings() {
    // Save settings to QSettings
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());

    m_settings->setValue("symbol", m_symbolEdit->text());
    m_settings->setValue("model", m_modelCombo->currentText());
}

QString MainWindow::formatCurrency(double value) const {
    return QString("$%1").arg(value, 0, 'f', 2);
}

QString MainWindow::formatPercentage(double value) const {
    return QString("%1%").arg(value * 100, 0, 'f', 2);
}

QString MainWindow::formatDateTime(const std::chrono::system_clock::time_point& time) const {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    return QDateTime::fromSecsSinceEpoch(time_t).toString();
}

// Worker class implementations
PredictionWorker::PredictionWorker(QObject* parent) : QObject(parent), m_predictor(nullptr) {}

void PredictionWorker::trainModel(const std::vector<stock_predict::MarketData>& data,
                                  const stock_predict::PredictorConfig& config, int epochs) {
    QMutexLocker locker(&m_mutex);

    try {
        m_predictor = stock_predict::create_predictor("WORKER", config);

        bool success = m_predictor->train(data, epochs);
        emit trainingComplete(success,
                              success ? "Training completed successfully" : "Training failed");
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void PredictionWorker::makePrediction(const std::vector<stock_predict::MarketData>& data,
                                      const stock_predict::PredictorConfig& config, int days) {
    QMutexLocker locker(&m_mutex);

    try {
        if (!m_predictor) {
            m_predictor = stock_predict::create_predictor("WORKER", config);
        }

        auto result = m_predictor->predict_next_day(data);
        emit predictionReady(result);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

void PredictionWorker::runBacktest(const std::vector<stock_predict::MarketData>& data,
                                   const stock_predict::PredictorConfig& config,
                                   double initialCapital) {
    QMutexLocker locker(&m_mutex);

    try {
        // Implement backtesting logic
        QVariantMap results;
        results["totalReturn"] = 0.156;
        results["sharpeRatio"] = 1.23;
        results["maxDrawdown"] = -0.084;

        emit backtestComplete(results);
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

// Real-time updater implementations
RealTimeUpdater::RealTimeUpdater(QObject* parent)
    : QObject(parent),
      m_timer(new QTimer(this)),
      m_dataManager(std::make_unique<stock_predict::DataManager>()),
      m_isRunning(false) {
    connect(m_timer, &QTimer::timeout, this, &RealTimeUpdater::updateData);
}

void RealTimeUpdater::setSymbol(const QString& symbol) {
    QMutexLocker locker(&m_mutex);
    m_symbol = symbol;
}

void RealTimeUpdater::setUpdateInterval(int milliseconds) { m_timer->setInterval(milliseconds); }

void RealTimeUpdater::start() {
    QMutexLocker locker(&m_mutex);

    if (!m_isRunning) {
        m_isRunning = true;
        m_timer->start();
    }
}

void RealTimeUpdater::stop() {
    QMutexLocker locker(&m_mutex);

    if (m_isRunning) {
        m_isRunning = false;
        m_timer->stop();
    }
}

void RealTimeUpdater::updateData() {
    QMutexLocker locker(&m_mutex);

    try {
        auto data = m_dataManager->get_real_time_data(m_symbol.toStdString());
        if (data.has_value()) {
            emit dataUpdated(data.value());
        }
    } catch (const std::exception& e) {
        emit errorOccurred(QString::fromStdString(e.what()));
    }
}

#include "main_window.moc"
#endif  // QT_AVAILABLE
