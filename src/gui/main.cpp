#include <QApplication>
#include "stock_predict/gui/main_window.hpp"
#include <QStyleFactory>
#include <QDirIterator>
#include <QDebug>

void setupApplicationStyle() {
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    // Dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    QApplication::setPalette(darkPalette);
    
    // Additional styling
    qApp->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #616161;
        }
        
        QTabBar::tab {
            background-color: #424242;
            color: white;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        
        QTabBar::tab:selected {
            background-color: #2196F3;
        }
        
        QTabBar::tab:hover {
            background-color: #616161;
        }
        
        QGroupBox {
            font-weight: bold;
            border: 2px solid #616161;
            border-radius: 5px;
            margin-top: 1ex;
            padding-top: 10px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 5px;
        }
        
        QPushButton {
            border: 1px solid #616161;
            border-radius: 4px;
            padding: 6px;
            min-width: 80px;
        }
        
        QPushButton:hover {
            border: 1px solid #2196F3;
        }
        
        QPushButton:pressed {
            background-color: #1976D2;
        }
        
        QTableWidget {
            gridline-color: #616161;
            selection-background-color: #2196F3;
        }
        
        QHeaderView::section {
            background-color: #424242;
            color: white;
            border: 1px solid #616161;
            padding: 4px;
            font-weight: bold;
        }
        
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            border: 1px solid #616161;
            border-radius: 3px;
            padding: 4px;
            background-color: #353535;
        }
        
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border: 2px solid #2196F3;
        }
        
        QScrollBar:vertical {
            background-color: #424242;
            width: 12px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #616161;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #2196F3;
        }
    )");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("Stock Predict");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("StockPredict");
    app.setOrganizationDomain("stockpredict.com");
    
    // Setup application style
    setupApplicationStyle();
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
