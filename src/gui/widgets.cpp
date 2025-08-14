#ifdef QT_AVAILABLE
#include <QtCore/QSettings>
#include <QtCore/QStandardItemModel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <cmath>
#include "stock_predict/gui/main_window.hpp"

namespace stock_predict {
namespace gui {

// PortfolioWidget Implementation
PortfolioWidget::PortfolioWidget(QWidget* parent)
    : QWidget(parent),
      portfolio_table_(nullptr),
      portfolio_model_(nullptr),
      total_value_label_(nullptr),
      total_return_label_(nullptr),
      total_risk_label_(nullptr) {
    setupUI();
    setupConnections();
    loadSampleData();
}

void PortfolioWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Summary section
    auto* summary_group = new QGroupBox("Portfolio Summary");
    auto* summary_layout = new QGridLayout(summary_group);

    summary_layout->addWidget(new QLabel("Total Value:"), 0, 0);
    total_value_label_ = new QLabel("$0.00");
    total_value_label_->setStyleSheet("font-weight: bold; color: #2E7D32;");
    summary_layout->addWidget(total_value_label_, 0, 1);

    summary_layout->addWidget(new QLabel("Total Return:"), 0, 2);
    total_return_label_ = new QLabel("0.00%");
    total_return_label_->setStyleSheet("font-weight: bold;");
    summary_layout->addWidget(total_return_label_, 0, 3);

    summary_layout->addWidget(new QLabel("Total Risk (VaR):"), 1, 0);
    total_risk_label_ = new QLabel("$0.00");
    total_risk_label_->setStyleSheet("font-weight: bold; color: #C62828;");
    summary_layout->addWidget(total_risk_label_, 1, 1);

    layout->addWidget(summary_group);

    // Holdings table
    auto* holdings_group = new QGroupBox("Holdings");
    auto* holdings_layout = new QVBoxLayout(holdings_group);

    portfolio_model_ = new QStandardItemModel(this);
    portfolio_model_->setHorizontalHeaderLabels(
        {"Symbol", "Shares", "Price", "Value", "Return", "Return %", "Weight", "Risk"});

    portfolio_table_ = new QTableView();
    portfolio_table_->setModel(portfolio_model_);
    portfolio_table_->setAlternatingRowColors(true);
    portfolio_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    portfolio_table_->horizontalHeader()->setStretchLastSection(true);
    portfolio_table_->setSortingEnabled(true);

    holdings_layout->addWidget(portfolio_table_);

    // Action buttons
    auto* button_layout = new QHBoxLayout();

    auto* add_holding_btn = new QPushButton("Add Position");
    auto* remove_holding_btn = new QPushButton("Remove Position");
    auto* optimize_btn = new QPushButton("Optimize Portfolio");
    auto* rebalance_btn = new QPushButton("Rebalance");
    auto* export_btn = new QPushButton("Export");

    button_layout->addWidget(add_holding_btn);
    button_layout->addWidget(remove_holding_btn);
    button_layout->addStretch();
    button_layout->addWidget(optimize_btn);
    button_layout->addWidget(rebalance_btn);
    button_layout->addWidget(export_btn);

    holdings_layout->addLayout(button_layout);
    layout->addWidget(holdings_group);
}

void PortfolioWidget::setupConnections() {
    // Connections would be implemented here
}

void PortfolioWidget::loadSampleData() {
    // Sample portfolio data
    struct Position {
        QString symbol;
        int shares;
        double price;
        double cost_basis;
    };

    std::vector<Position> positions = {{"AAPL", 100, 175.43, 150.00},
                                       {"GOOGL", 50, 2650.23, 2500.00},
                                       {"MSFT", 75, 331.62, 300.00},
                                       {"AMZN", 25, 3312.17, 3200.00},
                                       {"TSLA", 40, 780.79, 850.00}};

    portfolio_model_->clear();
    portfolio_model_->setHorizontalHeaderLabels(
        {"Symbol", "Shares", "Price", "Value", "Return", "Return %", "Weight", "Risk"});

    double total_value = 0.0;
    double total_cost = 0.0;

    for (const auto& pos : positions) {
        double value = pos.shares * pos.price;
        double cost = pos.shares * pos.cost_basis;
        double return_abs = value - cost;
        double return_pct = (return_abs / cost) * 100.0;

        total_value += value;
        total_cost += cost;

        auto* row = new QList<QStandardItem*>();
        row->append(new QStandardItem(pos.symbol));
        row->append(new QStandardItem(QString::number(pos.shares)));
        row->append(new QStandardItem(QString("$%1").arg(pos.price, 0, 'f', 2)));
        row->append(new QStandardItem(QString("$%1").arg(value, 0, 'f', 2)));
        row->append(new QStandardItem(QString("$%1").arg(return_abs, 0, 'f', 2)));

        auto* return_item = new QStandardItem(QString("%1%").arg(return_pct, 0, 'f', 2));
        if (return_pct >= 0) {
            return_item->setForeground(QBrush(QColor(46, 125, 50)));  // Green
        } else {
            return_item->setForeground(QBrush(QColor(198, 40, 40)));  // Red
        }
        row->append(return_item);

        row->append(new QStandardItem("0.0%"));   // Weight - calculated after
        row->append(new QStandardItem("$0.00"));  // Risk - placeholder

        portfolio_model_->appendRow(*row);
        delete row;
    }

    // Update weights
    for (int i = 0; i < portfolio_model_->rowCount(); ++i) {
        double value = portfolio_model_->item(i, 3)->text().remove("$").toDouble();
        double weight = (value / total_value) * 100.0;
        portfolio_model_->setItem(i, 6, new QStandardItem(QString("%1%").arg(weight, 0, 'f', 1)));
    }

    // Update summary
    total_value_label_->setText(QString("$%1").arg(total_value, 0, 'f', 2));

    double total_return_pct = ((total_value - total_cost) / total_cost) * 100.0;
    total_return_label_->setText(QString("%1%").arg(total_return_pct, 0, 'f', 2));
    if (total_return_pct >= 0) {
        total_return_label_->setStyleSheet("font-weight: bold; color: #2E7D32;");
    } else {
        total_return_label_->setStyleSheet("font-weight: bold; color: #C62828;");
    }

    total_risk_label_->setText("$12,450.00");  // Placeholder VaR
}

void PortfolioWidget::updatePortfolio() {
    loadSampleData();  // Refresh data
}

void PortfolioWidget::addPosition(const QString& symbol, int shares, double price) {
    // Add new position implementation
}

void PortfolioWidget::removePosition(int row) {
    if (row >= 0 && row < portfolio_model_->rowCount()) {
        portfolio_model_->removeRow(row);
        updatePortfolio();
    }
}

// RiskWidget Implementation
RiskWidget::RiskWidget(QWidget* parent)
    : QWidget(parent),
      risk_metrics_table_(nullptr),
      risk_model_(nullptr),
      var_95_label_(nullptr),
      var_99_label_(nullptr),
      expected_shortfall_label_(nullptr),
      beta_label_(nullptr),
      sharpe_ratio_label_(nullptr),
      max_drawdown_label_(nullptr) {
    setupUI();
    setupConnections();
    loadSampleRiskData();
}

void RiskWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Risk summary section
    auto* summary_group = new QGroupBox("Risk Summary");
    auto* summary_layout = new QGridLayout(summary_group);

    // Row 0
    summary_layout->addWidget(new QLabel("VaR (95%):"), 0, 0);
    var_95_label_ = new QLabel("$0.00");
    var_95_label_->setStyleSheet("font-weight: bold; color: #FF5722;");
    summary_layout->addWidget(var_95_label_, 0, 1);

    summary_layout->addWidget(new QLabel("VaR (99%):"), 0, 2);
    var_99_label_ = new QLabel("$0.00");
    var_99_label_->setStyleSheet("font-weight: bold; color: #D32F2F;");
    summary_layout->addWidget(var_99_label_, 0, 3);

    // Row 1
    summary_layout->addWidget(new QLabel("Expected Shortfall:"), 1, 0);
    expected_shortfall_label_ = new QLabel("$0.00");
    expected_shortfall_label_->setStyleSheet("font-weight: bold; color: #C62828;");
    summary_layout->addWidget(expected_shortfall_label_, 1, 1);

    summary_layout->addWidget(new QLabel("Beta:"), 1, 2);
    beta_label_ = new QLabel("0.00");
    beta_label_->setStyleSheet("font-weight: bold;");
    summary_layout->addWidget(beta_label_, 1, 3);

    // Row 2
    summary_layout->addWidget(new QLabel("Sharpe Ratio:"), 2, 0);
    sharpe_ratio_label_ = new QLabel("0.00");
    sharpe_ratio_label_->setStyleSheet("font-weight: bold; color: #388E3C;");
    summary_layout->addWidget(sharpe_ratio_label_, 2, 1);

    summary_layout->addWidget(new QLabel("Max Drawdown:"), 2, 2);
    max_drawdown_label_ = new QLabel("0.00%");
    max_drawdown_label_->setStyleSheet("font-weight: bold; color: #F57C00;");
    summary_layout->addWidget(max_drawdown_label_, 2, 3);

    layout->addWidget(summary_group);

    // Detailed risk metrics table
    auto* details_group = new QGroupBox("Detailed Risk Metrics");
    auto* details_layout = new QVBoxLayout(details_group);

    risk_model_ = new QStandardItemModel(this);
    risk_model_->setHorizontalHeaderLabels(
        {"Metric", "Current", "1-Day", "1-Week", "1-Month", "1-Year"});

    risk_metrics_table_ = new QTableView();
    risk_metrics_table_->setModel(risk_model_);
    risk_metrics_table_->setAlternatingRowColors(true);
    risk_metrics_table_->horizontalHeader()->setStretchLastSection(true);
    risk_metrics_table_->verticalHeader()->setVisible(false);

    details_layout->addWidget(risk_metrics_table_);

    // Risk control buttons
    auto* button_layout = new QHBoxLayout();

    auto* calculate_risk_btn = new QPushButton("Calculate Risk");
    auto* monte_carlo_btn = new QPushButton("Monte Carlo Simulation");
    auto* stress_test_btn = new QPushButton("Stress Test");
    auto* correlation_btn = new QPushButton("Correlation Analysis");
    auto* export_risk_btn = new QPushButton("Export Risk Report");

    button_layout->addWidget(calculate_risk_btn);
    button_layout->addWidget(monte_carlo_btn);
    button_layout->addWidget(stress_test_btn);
    button_layout->addWidget(correlation_btn);
    button_layout->addStretch();
    button_layout->addWidget(export_risk_btn);

    details_layout->addLayout(button_layout);
    layout->addWidget(details_group);
}

void RiskWidget::setupConnections() {
    // Connections would be implemented here
}

void RiskWidget::loadSampleRiskData() {
    // Sample risk metrics
    struct RiskMetric {
        QString name;
        QString current;
        QString one_day;
        QString one_week;
        QString one_month;
        QString one_year;
    };

    std::vector<RiskMetric> metrics = {
        {"Volatility", "18.5%", "2.1%", "8.3%", "15.2%", "22.1%"},
        {"Value at Risk (95%)", "$8,250", "$1,200", "$4,800", "$8,250", "$15,600"},
        {"Value at Risk (99%)", "$12,450", "$1,800", "$7,200", "$12,450", "$23,400"},
        {"Expected Shortfall", "$15,200", "$2,200", "$8,800", "$15,200", "$28,600"},
        {"Maximum Drawdown", "-12.3%", "-2.1%", "-6.8%", "-12.3%", "-18.9%"},
        {"Sharpe Ratio", "1.42", "N/A", "1.38", "1.42", "1.35"},
        {"Beta", "1.15", "N/A", "1.12", "1.15", "1.18"},
        {"Alpha", "2.8%", "N/A", "2.5%", "2.8%", "3.2%"},
        {"Information Ratio", "0.85", "N/A", "0.82", "0.85", "0.88"},
        {"Tracking Error", "4.2%", "N/A", "3.8%", "4.2%", "4.6%"}};

    risk_model_->clear();
    risk_model_->setHorizontalHeaderLabels(
        {"Metric", "Current", "1-Day", "1-Week", "1-Month", "1-Year"});

    for (const auto& metric : metrics) {
        auto* row = new QList<QStandardItem*>();
        row->append(new QStandardItem(metric.name));
        row->append(new QStandardItem(metric.current));
        row->append(new QStandardItem(metric.one_day));
        row->append(new QStandardItem(metric.one_week));
        row->append(new QStandardItem(metric.one_month));
        row->append(new QStandardItem(metric.one_year));

        risk_model_->appendRow(*row);
        delete row;
    }

    // Update summary labels
    var_95_label_->setText("$8,250");
    var_99_label_->setText("$12,450");
    expected_shortfall_label_->setText("$15,200");
    beta_label_->setText("1.15");
    sharpe_ratio_label_->setText("1.42");
    max_drawdown_label_->setText("-12.3%");
}

void RiskWidget::updateRiskMetrics() {
    loadSampleRiskData();  // Refresh risk data
}

void RiskWidget::calculateVaR(double confidence_level) {
    // VaR calculation implementation
}

void RiskWidget::runMonteCarloSimulation(int num_simulations, int time_horizon) {
    // Monte Carlo simulation implementation
}

// SettingsDialog Implementation
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setModal(true);
    resize(500, 400);

    setupUI();
    setupConnections();
    loadSettings();
}

void SettingsDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Create tabs
    auto* tabs = new QTabWidget();

    // General tab
    auto* general_tab = createGeneralTab();
    tabs->addTab(general_tab, "General");

    // Model tab
    auto* model_tab = createModelTab();
    tabs->addTab(model_tab, "Model");

    // Data tab
    auto* data_tab = createDataTab();
    tabs->addTab(data_tab, "Data");

    // Risk tab
    auto* risk_tab = createRiskTab();
    tabs->addTab(risk_tab, "Risk");

    layout->addWidget(tabs);

    // Buttons
    auto* button_layout = new QHBoxLayout();

    auto* defaults_btn = new QPushButton("Restore Defaults");
    auto* cancel_btn = new QPushButton("Cancel");
    auto* ok_btn = new QPushButton("OK");

    ok_btn->setDefault(true);

    button_layout->addWidget(defaults_btn);
    button_layout->addStretch();
    button_layout->addWidget(cancel_btn);
    button_layout->addWidget(ok_btn);

    layout->addLayout(button_layout);

    // Connect buttons
    connect(ok_btn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    connect(defaults_btn, &QPushButton::clicked, this, &SettingsDialog::restoreDefaults);
}

QWidget* SettingsDialog::createGeneralTab() {
    auto* widget = new QWidget();
    auto* layout = new QFormLayout(widget);

    // UI settings
    auto* ui_group = new QGroupBox("User Interface");
    auto* ui_layout = new QFormLayout(ui_group);

    theme_combo_ = new QComboBox();
    theme_combo_->addItems({"Light", "Dark", "Auto"});
    ui_layout->addRow("Theme:", theme_combo_);

    auto_update_checkbox_ = new QCheckBox();
    ui_layout->addRow("Auto-update data:", auto_update_checkbox_);

    update_interval_spin_ = new QSpinBox();
    update_interval_spin_->setRange(1, 3600);
    update_interval_spin_->setSuffix(" seconds");
    update_interval_spin_->setValue(30);
    ui_layout->addRow("Update interval:", update_interval_spin_);

    layout->addRow(ui_group);

    // Performance settings
    auto* perf_group = new QGroupBox("Performance");
    auto* perf_layout = new QFormLayout(perf_group);

    num_threads_spin_ = new QSpinBox();
    num_threads_spin_->setRange(1, 16);
    num_threads_spin_->setValue(4);
    perf_layout->addRow("Number of threads:", num_threads_spin_);

    use_gpu_checkbox_ = new QCheckBox();
    perf_layout->addRow("Use GPU acceleration:", use_gpu_checkbox_);

    layout->addRow(perf_group);

    return widget;
}

QWidget* SettingsDialog::createModelTab() {
    auto* widget = new QWidget();
    auto* layout = new QFormLayout(widget);

    // Model configuration
    auto* model_group = new QGroupBox("Model Configuration");
    auto* model_layout = new QFormLayout(model_group);

    default_model_combo_ = new QComboBox();
    default_model_combo_->addItems({"ensemble", "lstm", "transformer", "xgboost"});
    model_layout->addRow("Default model:", default_model_combo_);

    sequence_length_spin_ = new QSpinBox();
    sequence_length_spin_->setRange(10, 500);
    sequence_length_spin_->setValue(60);
    model_layout->addRow("Sequence length:", sequence_length_spin_);

    prediction_horizon_spin_ = new QSpinBox();
    prediction_horizon_spin_->setRange(1, 30);
    prediction_horizon_spin_->setValue(1);
    model_layout->addRow("Prediction horizon:", prediction_horizon_spin_);

    batch_size_spin_ = new QSpinBox();
    batch_size_spin_->setRange(1, 1024);
    batch_size_spin_->setValue(32);
    model_layout->addRow("Batch size:", batch_size_spin_);

    learning_rate_spin_ = new QDoubleSpinBox();
    learning_rate_spin_->setRange(0.00001, 1.0);
    learning_rate_spin_->setDecimals(5);
    learning_rate_spin_->setValue(0.001);
    model_layout->addRow("Learning rate:", learning_rate_spin_);

    layout->addRow(model_group);

    return widget;
}

QWidget* SettingsDialog::createDataTab() {
    auto* widget = new QWidget();
    auto* layout = new QFormLayout(widget);

    // Data sources
    auto* sources_group = new QGroupBox("Data Sources");
    auto* sources_layout = new QFormLayout(sources_group);

    primary_source_combo_ = new QComboBox();
    primary_source_combo_->addItems({"Yahoo Finance", "Alpha Vantage", "IEX Cloud", "Quandl"});
    sources_layout->addRow("Primary source:", primary_source_combo_);

    api_key_edit_ = new QLineEdit();
    api_key_edit_->setEchoMode(QLineEdit::Password);
    sources_layout->addRow("API Key:", api_key_edit_);

    cache_data_checkbox_ = new QCheckBox();
    cache_data_checkbox_->setChecked(true);
    sources_layout->addRow("Cache data locally:", cache_data_checkbox_);

    cache_duration_spin_ = new QSpinBox();
    cache_duration_spin_->setRange(1, 365);
    cache_duration_spin_->setSuffix(" days");
    cache_duration_spin_->setValue(7);
    sources_layout->addRow("Cache duration:", cache_duration_spin_);

    layout->addRow(sources_group);

    return widget;
}

QWidget* SettingsDialog::createRiskTab() {
    auto* widget = new QWidget();
    auto* layout = new QFormLayout(widget);

    // Risk parameters
    auto* risk_group = new QGroupBox("Risk Parameters");
    auto* risk_layout = new QFormLayout(risk_group);

    var_confidence_spin_ = new QDoubleSpinBox();
    var_confidence_spin_->setRange(0.90, 0.99);
    var_confidence_spin_->setDecimals(2);
    var_confidence_spin_->setValue(0.95);
    risk_layout->addRow("VaR confidence level:", var_confidence_spin_);

    monte_carlo_runs_spin_ = new QSpinBox();
    monte_carlo_runs_spin_->setRange(1000, 1000000);
    monte_carlo_runs_spin_->setValue(10000);
    risk_layout->addRow("Monte Carlo runs:", monte_carlo_runs_spin_);

    risk_free_rate_spin_ = new QDoubleSpinBox();
    risk_free_rate_spin_->setRange(0.0, 0.10);
    risk_free_rate_spin_->setDecimals(4);
    risk_free_rate_spin_->setValue(0.02);
    risk_layout->addRow("Risk-free rate:", risk_free_rate_spin_);

    layout->addRow(risk_group);

    return widget;
}

void SettingsDialog::setupConnections() {
    // Connections implementation
}

void SettingsDialog::loadSettings() {
    QSettings settings;

    // Load saved settings
    theme_combo_->setCurrentText(settings.value("theme", "Light").toString());
    auto_update_checkbox_->setChecked(settings.value("auto_update", true).toBool());
    update_interval_spin_->setValue(settings.value("update_interval", 30).toInt());
    use_gpu_checkbox_->setChecked(settings.value("use_gpu", false).toBool());

    default_model_combo_->setCurrentText(settings.value("default_model", "ensemble").toString());
    sequence_length_spin_->setValue(settings.value("sequence_length", 60).toInt());
    prediction_horizon_spin_->setValue(settings.value("prediction_horizon", 1).toInt());

    primary_source_combo_->setCurrentText(
        settings.value("primary_source", "Yahoo Finance").toString());
    cache_data_checkbox_->setChecked(settings.value("cache_data", true).toBool());

    var_confidence_spin_->setValue(settings.value("var_confidence", 0.95).toDouble());
    monte_carlo_runs_spin_->setValue(settings.value("monte_carlo_runs", 10000).toInt());
}

void SettingsDialog::saveSettings() {
    QSettings settings;

    // Save settings
    settings.setValue("theme", theme_combo_->currentText());
    settings.setValue("auto_update", auto_update_checkbox_->isChecked());
    settings.setValue("update_interval", update_interval_spin_->value());
    settings.setValue("use_gpu", use_gpu_checkbox_->isChecked());

    settings.setValue("default_model", default_model_combo_->currentText());
    settings.setValue("sequence_length", sequence_length_spin_->value());
    settings.setValue("prediction_horizon", prediction_horizon_spin_->value());

    settings.setValue("primary_source", primary_source_combo_->currentText());
    settings.setValue("cache_data", cache_data_checkbox_->isChecked());

    settings.setValue("var_confidence", var_confidence_spin_->value());
    settings.setValue("monte_carlo_runs", monte_carlo_runs_spin_->value());
}

void SettingsDialog::restoreDefaults() {
    // Restore default values
    theme_combo_->setCurrentText("Light");
    auto_update_checkbox_->setChecked(true);
    update_interval_spin_->setValue(30);
    use_gpu_checkbox_->setChecked(false);

    default_model_combo_->setCurrentText("ensemble");
    sequence_length_spin_->setValue(60);
    prediction_horizon_spin_->setValue(1);

    primary_source_combo_->setCurrentText("Yahoo Finance");
    cache_data_checkbox_->setChecked(true);

    var_confidence_spin_->setValue(0.95);
    monte_carlo_runs_spin_->setValue(10000);
}

void SettingsDialog::accept() {
    saveSettings();
    QDialog::accept();
}

// AboutDialog Implementation
AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("About Stock Prediction System");
    setModal(true);
    setFixedSize(450, 350);

    setupUI();
}

void AboutDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // Logo and title
    auto* header_layout = new QHBoxLayout();

    auto* logo_label = new QLabel();
    logo_label->setPixmap(QPixmap(":/icons/app_icon.png")
                              .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    header_layout->addWidget(logo_label);

    auto* title_layout = new QVBoxLayout();
    auto* title_label = new QLabel("Stock Prediction System");
    title_label->setStyleSheet("font-size: 18px; font-weight: bold;");
    auto* version_label = new QLabel("Version 1.0.0");
    version_label->setStyleSheet("color: #666666;");

    title_layout->addWidget(title_label);
    title_layout->addWidget(version_label);
    title_layout->addStretch();

    header_layout->addLayout(title_layout);
    header_layout->addStretch();

    layout->addLayout(header_layout);

    // Description
    auto* desc_label = new QLabel(
        "Advanced C++ stock price prediction system using machine learning "
        "algorithms including LSTM, Transformer, and Ensemble models. "
        "Features comprehensive risk analysis, portfolio management, and "
        "real-time data processing capabilities.");
    desc_label->setWordWrap(true);
    desc_label->setStyleSheet("margin: 10px 0px;");
    layout->addWidget(desc_label);

    // Technical info
    auto* tech_group = new QGroupBox("Technical Information");
    auto* tech_layout = new QVBoxLayout(tech_group);

    tech_layout->addWidget(new QLabel("• Built with C++20 and Qt6"));
    tech_layout->addWidget(new QLabel("• Pure C++ mathematical implementations"));
    tech_layout->addWidget(new QLabel("• Eigen3 for mathematical computations"));
    tech_layout->addWidget(new QLabel("• HDF5 for efficient data storage"));
    tech_layout->addWidget(new QLabel("• Qt Charts for visualization"));

    layout->addWidget(tech_group);

    // Copyright and license
    auto* copyright_label = new QLabel(
        "Copyright © 2024 Stock Prediction System\n"
        "Licensed under MIT License");
    copyright_label->setAlignment(Qt::AlignCenter);
    copyright_label->setStyleSheet("color: #666666; font-size: 11px; margin: 10px 0px;");
    layout->addWidget(copyright_label);

    // Close button
    auto* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    auto* close_btn = new QPushButton("Close");
    close_btn->setDefault(true);
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);

    button_layout->addWidget(close_btn);
    layout->addLayout(button_layout);
}

}  // namespace gui
}  // namespace stock_predict
#endif  // QT_AVAILABLE
