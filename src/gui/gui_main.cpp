#ifdef QT_AVAILABLE
#include <QtCore/QCommandLineParser>
#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QStandardPaths>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QSplashScreen>
#include <memory>

#include "stock_predict/gui/main_window.h"
#include "stock_predict/predictor.hpp"

Q_LOGGING_CATEGORY(main, "stockpredict.main")

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Stock Prediction System");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("Stock Predict - Advanced Trading System");
    app.setOrganizationName("StockPredict");
    app.setOrganizationDomain("stockpredict.com");

    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Advanced C++ Stock Prediction System with Professional GUI");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption symbolOption(QStringList() << "s" << "symbol",
                                    "Initial stock symbol to load (default: AAPL)", "symbol",
                                    "AAPL");
    parser.addOption(symbolOption);

    QCommandLineOption modelOption(QStringList() << "m" << "model",
                                   "Initial model type (ensemble, lstm, transformer)", "model",
                                   "ensemble");
    parser.addOption(modelOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Enable debug output");
    parser.addOption(debugOption);

    QCommandLineOption gpuOption(QStringList() << "g" << "gpu", "Enable GPU acceleration");
    parser.addOption(gpuOption);

    // Process command line arguments
    parser.process(app);

    // Enable debug logging if requested
    if (parser.isSet(debugOption)) {
        QLoggingCategory::setFilterRules("stockpredict.*=true");
    }

    // Create splash screen
    QPixmap splashPixmap(":/images/splash.png");
    if (splashPixmap.isNull()) {
        // Create a simple splash screen if image is not available
        splashPixmap = QPixmap(400, 300);
        splashPixmap.fill(Qt::white);
    }

    QSplashScreen splash(splashPixmap);
    splash.show();
    app.processEvents();

    splash.showMessage("Initializing Stock Prediction System...", Qt::AlignBottom | Qt::AlignCenter,
                       Qt::black);
    app.processEvents();

    try {
        // Initialize the main window
        splash.showMessage("Loading main interface...", Qt::AlignBottom | Qt::AlignCenter,
                           Qt::black);
        app.processEvents();

        stock_predict::gui::MainWindow window;

        // Apply command line options
        if (parser.isSet(symbolOption)) {
            // Set initial symbol - this would be implemented in the window
            qCDebug(main) << "Setting initial symbol:" << parser.value(symbolOption);
        }

        if (parser.isSet(modelOption)) {
            // Set initial model - this would be implemented in the window
            qCDebug(main) << "Setting initial model:" << parser.value(modelOption);
        }

        if (parser.isSet(gpuOption)) {
            // Enable GPU - this would be implemented in the window
            qCDebug(main) << "GPU acceleration enabled";
        }

        splash.showMessage("Ready!", Qt::AlignBottom | Qt::AlignCenter, Qt::black);
        app.processEvents();

        // Show the main window
        window.show();
        splash.finish(&window);

        qCInfo(main) << "Stock Prediction System started successfully";

        // Run the application
        return app.exec();

    } catch (const std::exception& e) {
        splash.close();

        QString errorMsg = QString("Failed to initialize application: %1").arg(e.what());
        qCCritical(main) << errorMsg;

        QMessageBox::critical(nullptr, "Startup Error", errorMsg);
        return 1;
    } catch (...) {
        splash.close();

        QString errorMsg = "Unknown error occurred during startup";
        qCCritical(main) << errorMsg;

        QMessageBox::critical(nullptr, "Startup Error", errorMsg);
        return 1;
    }
}
#endif // QT_AVAILABLE
