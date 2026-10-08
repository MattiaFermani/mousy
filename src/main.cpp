#include <QApplication>
#include "MainWindow.h"
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set a modern global font
    QFont font("Segoe UI", 10);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);
    
    // Apply an Ultra-Modern Dark Theme
    QString style = R"(
        QMainWindow {
            background-color: #121212;
        }
        QTabWidget::pane {
            border: 1px solid #2a2a2a;
            background-color: #181818;
            border-radius: 10px;
        }
        QTabBar::tab {
            background-color: #202020;
            color: #888888;
            padding: 12px 24px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            margin-right: 4px;
            font-weight: bold;
            font-size: 14px;
        }
        QTabBar::tab:selected {
            background-color: #181818;
            color: #ffffff;
            border-bottom: 3px solid #00d2ff;
        }
        QTabBar::tab:hover:!selected {
            background-color: #2a2a2a;
            color: #cccccc;
        }
        QLabel {
            color: #e0e0e0;
            font-size: 14px;
        }
        QLabel h2 {
            color: #ffffff;
            font-size: 24px;
        }
        QPushButton {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #3a7bd5, stop: 1 #3a6073);
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 8px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #00d2ff, stop: 1 #3a7bd5);
        }
        QPushButton:pressed {
            background-color: #2a5298;
        }
        QPushButton:disabled {
            background-color: #2a2a2a;
            color: #555555;
        }
        QWidget#ConfigPanel {
            background-color: #181818;
            border-left: 1px solid #2a2a2a;
        }
    )";
    
    app.setStyleSheet(style);
    
    MainWindow w;
    w.show();
    
    return app.exec();
}
