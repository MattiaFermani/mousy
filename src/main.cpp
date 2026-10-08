#include <QApplication>
#include "MainWindow.h"
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Modern global font
    QFont font("Segoe UI", 10);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);
    
    // Apply an Ultra-Modern Gaming Dark Theme
    QString style = R"(
        QMainWindow, QDialog, QMessageBox {
            background-color: #101014;
            color: #e0e0e0;
        }
        QTabWidget::pane {
            border: 1px solid #22222a;
            background-color: #14141a;
            border-radius: 10px;
        }
        QTabBar::tab {
            background-color: #1a1a22;
            color: #888899;
            padding: 12px 24px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            margin-right: 4px;
            font-weight: bold;
            font-size: 13px;
        }
        QTabBar::tab:selected {
            background-color: #14141a;
            color: #ffffff;
            border-bottom: 3px solid #00d2ff;
        }
        QTabBar::tab:hover:!selected {
            background-color: #242430;
            color: #cccccc;
        }
        QLabel {
            color: #e0e0e0;
        }
        QPushButton {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #2a5298, stop: 1 #1e3c72);
            color: white;
            border: 1px solid #3060a0;
            padding: 10px 20px;
            border-radius: 7px;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #00b4db, stop: 1 #0083b0);
            border-color: #00d2ff;
        }
        QPushButton:pressed {
            background-color: #173760;
        }
        QPushButton:disabled {
            background-color: #1f1f26;
            color: #555566;
            border: 1px solid #282832;
        }
        QComboBox {
            background-color: #1c1c24;
            color: #ffffff;
            border: 1px solid #2e2e3c;
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 13px;
            min-height: 20px;
        }
        QComboBox:hover, QComboBox:focus {
            border: 1px solid #00d2ff;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1px solid #282835;
        }
        QComboBox QAbstractItemView {
            background-color: #181820;
            color: #e0e0e0;
            selection-background-color: #1a3a5c;
            selection-color: #00d2ff;
            border: 1px solid #2e2e3c;
            outline: none;
            padding: 4px;
        }
        QSpinBox, QLineEdit {
            background-color: #1c1c24;
            color: #ffffff;
            border: 1px solid #2e2e3c;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 13px;
        }
        QSpinBox:focus, QLineEdit:focus {
            border: 1px solid #00d2ff;
        }
        QCheckBox {
            color: #ccc;
            font-size: 13px;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 1px solid #333345;
            background: #1c1c24;
        }
        QCheckBox::indicator:checked {
            background: #00d2ff;
            border-color: #00d2ff;
        }
        QScrollBar:vertical {
            background: #14141a;
            width: 10px;
            margin: 0;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #2a2a38;
            min-height: 25px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical:hover {
            background: #00d2ff;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
    
    app.setStyleSheet(style);
    
    MainWindow w;
    w.show();
    
    return app.exec();
}
